#include "assets/loaders/font_loader.h"

#include "core/files.h"
#include "core/debugger.h"
#include <collections/DynamicArray.h>
#include <math/vec2.h>
#include "renderer/render_types.h"

#include "assets/asset_manager.h"

#include <stdlib.h>
#include <stdio.h>

// TODO: use the renderer front-end when ready
#include "renderer/details/texture.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

typedef struct GlyphSet_internal_t
{
    struct
    {
        u8 *pixels;
        u32 width, height;
    } atlas;

    struct
    {
        stbrp_context context; // Persistent packing context
        stbrp_node *nodes;     // Dynamically allocated nodes
        u32 node_count;        // Number of nodes
    } packer;
} GlyphSetInternal;

#define INTERNAL(set) ((GlyphSetInternal *)(set).__internal__)

Asset load_font_bitmap(const char *path, const char *name);
Asset load_font_ttf(const char *path);
Glyph *get_glyph(Font *font, u16 id, u32 size);
GlyphSet *create_glyphset(Font *tf, f32 size);
bool packGlyphIntoAtlas(GlyphSet *glyphSet, FT_Bitmap *bitmap, u32 *outX, u32 *outY);

static FT_Library *library = 0; // TODO: free this when shutting down the asset manager | move managing it to loader inti & shutdown

void init_font_loader()
{
}

void shutdown_font_loader()
{
    if (library)
    {
        FT_Done_FreeType(*library);
        free(library);
        library = 0;
    }
}

Asset load_font(const char *path, void *name_ptr)
{
    String ext = path_get_file_extension(path);
    if (str_equals_val(ext, "fnt"))
    {
        return load_font_bitmap(path, (char *)name_ptr);
    }
    else if (str_equals_val(ext, "ttf"))
    {
        return load_font_ttf(path);
    }
    else
    {
        LOG_ERROR("load_font() -> Unsupported font format: %s", ext.val);
        return (Asset){0};
    }
}

static u32 font_index = 0;
#include <assets/loaders/texture_loader.h>

/// @brief
/// @param path
/// @param name
/// @return
Asset load_font_bitmap(const char *path, const char *name)
{
    FileHandle *fntFile = file_load(path);
    if (!fntFile)
    {
        LOG_ERROR("load_font() -> file does not exist or could not be loaded.");
        return NULL_ASSET;
    }

    String pngFile = str_join(str_join(path_get_file_dir(path), path_get_file_name(path)), str_new(".png"));
    //  load_asset(, str_join(str_new(name), str_new("_atlas")).val);
    Asset atlas = load_texture((const char *)pngFile.val, &font_index);
    Vec2 atlas_size = vec2_new(((Texture *)atlas.data)->width, ((Texture *)atlas.data)->height);

    if (atlas.type != ASSET_TYPE_TEXTURE)
    {
        LOG_ERROR("load_asset() : failed to load font atlas %s", pngFile.val);
        return NULL_ASSET;
    }

    Asset *font_asset = get_asset(name, ASSET_TYPE_FONT);
    Font *font;
    if (!font_asset)
    {
        font = malloc(sizeof(Font));
        font->glyph_sets = DynamicArray_Create(GlyphSet);
    }
    else
    {
        font = (Font *)font_asset->data;
    }

    font->face = 0;
    Glyph *glyphs = DynamicArray_Create(Glyph);
    Glyph *glyph = malloc(sizeof(Glyph));
    f32 font_size = 0;
    for (u32 i = 0; i < fntFile->line_count; i++, file_next_line(fntFile))
    {
        String line = file_line_toString(fntFile);
        if (line.val[0] == 'c' && line.val[1] == 'o')
        {
            // parse common
        }
        else if (line.val[0] == 'i' && line.val[1] == 'n')
        {
            // parse info
            sscanf_s(line.val, "info%*[^s]size=%f", &font_size);
        }
        else if (line.val[0] == 'p' && line.val[1] == 'a')
        {
            // parse page
        }
        else if (line.val[0] == 'c' && line.val[1] == 'h' && line.val[4] == 's')
        {
            // parse chars
        }
        else if (line.val[0] == 'c' && line.val[1] == 'h' && line.val[4] == ' ')
        {
            // parse char
            *glyph = (Glyph){0};
            u32 page, chnl;
            sscanf_s(line.val, "char id=%hd x=%f y=%f width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d",
                     &glyph->id, &glyph->uv0.x, &glyph->uv0.y, &glyph->width, &glyph->height, &glyph->offset.x, &glyph->offset.y, &glyph->advance, &page, &chnl);
            glyph->uv1.x = (glyph->uv0.x + (f32)glyph->width) / atlas_size.x;
            glyph->uv1.y = (glyph->uv0.y + (f32)glyph->height) / atlas_size.y;
            glyph->uv0.x /= atlas_size.x;
            glyph->uv0.y /= atlas_size.y;
            DynamicArray_Push(glyphs, *glyph);
        }
        else if (line.val[0] == 'k' && line.val[1] == 'e')
        {
            // parse kerning
            // TODO: add kerning support
        }
    }

    GlyphSet __temp__ = {};
    __temp__.id = font_index++;
    __temp__.size = font_size;
    __temp__.atlas = atlas.data;
    const u32 glyphsCount = DynamicArray_Length(glyphs);
    __temp__.glyph_count = glyphsCount;
    // find the max glyph id
    for (u32 i = 0; i < glyphsCount; i++)
    {
        if (glyphs[i].id >= __temp__.glyph_count)
        {
            __temp__.glyph_count = glyphs[i].id + 1;
        }
    }
    // copy glyphs to a semi packed array
    __temp__.glyphs = malloc(sizeof(Glyph) * __temp__.glyph_count);
    for (u32 i = 0; i < __temp__.glyph_count; i++)
    {
        if (glyphs[i].id >= __temp__.glyph_count)
        {
            LOG_ERROR("load_font() -> glyph id %d is out of bounds", glyphs[i].id);
            continue;
        }
        __temp__.glyphs[glyphs[i].id] = glyphs[i];
    }
    DynamicArray_Destroy(glyphs);

    DynamicArray_Push(font->glyph_sets, __temp__);

    Asset new = {0};
    new.data = font;
    new.ref_count = 0;
    new.type = ASSET_TYPE_FONT;
    new.name = str_new(path);
    return new;
}

Asset load_font_ttf(const char *path)
{
    FT_Error error;
    if (!library)
    {
        library = malloc(sizeof(FT_Library));
        error = FT_Init_FreeType(library);
        if (error)
        {
            LOG_ERROR("load_font_ttf() -> Could not initialize FreeType library");
            return NULL_ASSET;
        }
    }

    Font *font = malloc(sizeof(Font));
    error = FT_New_Face(*library, path, 0, &font->face);

    if (error == FT_Err_Unknown_File_Format)
    {
        LOG_ERROR("load_font_ttf() -> Unsupported font format: %s", path_get_file_extension(path).val);
        FT_Done_FreeType(*library);
        return NULL_ASSET;
    }
    else if (error == FT_Err_Cannot_Open_Resource)
    {
        LOG_ERROR("load_font_ttf() -> Could not open font file: %s", path);
        FT_Done_FreeType(*library);
        return NULL_ASSET;
    }
    else if (error)
    {
        LOG_ERROR("load_font_ttf() -> Could not load font: %s", path);
        FT_Done_FreeType(*library);
        return NULL_ASSET;
    }
    font->glyph_sets = DynamicArray_Create(GlyphSet);
    Asset new = {0};
    new.data = font;
    new.ref_count = 0;
    new.type = ASSET_TYPE_FONT;
    new.name = str_new(path);
    return new;
}

void release_font(Asset *font)
{
    for (u32 i = 0; i < DynamicArray_Length(((Font *)font->data)->glyph_sets); i++)
    {
        GlyphSet set = ((Font *)font->data)->glyph_sets[i];
        if (set.atlas)
        {
            Texture *tex = (Texture *)set.atlas;
            destroyTexture(tex);
        }
        if (set.glyphs)
        {
            free(set.glyphs);
        }
    }
    if (((Font *)font->data)->face)
    {
        FT_Done_Face(((Font *)font->data)->face);
    }
    if (((Font *)font->data)->glyph_sets)
    {
        DynamicArray_Destroy(((Font *)font->data)->glyph_sets);
    }
    free((Font *)font->data);
}

GlyphSet *get_glyphset(Font *font, u32 size)
{
    if (!font)
    {
        LOG_ERROR("get_glyphset() -> Font is not initialized");
        return 0;
    }
    if (!font->glyph_sets)
    {
        LOG_TRACE("get_glyphset() -> no characters loaded yet for this font.");
        return 0;
    }
    // check if the char with the same size is already loaded
    for (u32 i = 0; i < DynamicArray_Length(font->glyph_sets); i++)
    {
        GlyphSet set = font->glyph_sets[i];
        if (set.size == size)
        {
            return &font->glyph_sets[i];
        }
    }
    return 0;
}

void load_char(Font *tf, char c, u32 size)
{
    if (!tf)
    {
        LOG_ERROR("load_char() -> Font is not initialized");
        return;
    }

    // check if the char with the same size is already loaded
    Glyph *ch = get_glyph(tf, c, size);
    if (ch)
    {
        return;
    }
    // if font is not a ttf return since fnt fonts load all chars at once
    if (!tf->face)
    {
        // LOG_WARN("load_char() -> Font is not a ttf font, cannot load char %c", c);
        return;
    }

    FT_Face face = tf->face;
    FT_Set_Pixel_Sizes(face, 0, size);
    u32 glyph_index = FT_Get_Char_Index(face, c);
    FT_Error error = FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER);
    if (error)
    {
        LOG_ERROR("load_char() -> Could not load glyph for char %c", c);
        return;
    }
    error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    if (error)
    {
        LOG_ERROR("load_char() -> Could not render glyph for char %c", c);
        return;
    }

    FT_Bitmap bitmap = face->glyph->bitmap;
    int width = bitmap.width;
    int height = bitmap.rows;

    GlyphSet *set = get_glyphset(tf, size);
    if (!set)
    {
        // if glyphset is not found create a new one
        set = create_glyphset(tf, size);
    }
    if (set->glyph_count <= c)
    {
        // resize the glyphs array to fit the new glyph
        Glyph *new_glyphs = malloc(sizeof(Glyph) * (c + 1));
        memcpy(new_glyphs, set->glyphs, sizeof(Glyph) * set->glyph_count);
        free(set->glyphs);
        set->glyphs = new_glyphs;
        set->glyph_count = c + 1;
    }

    // Pack the glyph into the atlas
    u32 x = 0, y = 0;
    if (width && height)
    {
        if (!packGlyphIntoAtlas(set, &bitmap, &x, &y))
        {
            LOG_ERROR("load_char() -> Failed to pack glyph %c into atlas", c);
            return;
        }

        Glyph glyph = {0};
        glyph.id = c;
        glyph.width = width;
        glyph.height = height;
        glyph.offset.x = face->glyph->bitmap_left;
        glyph.offset.y = set->size - face->glyph->bitmap_top; // Top of the glyph
        glyph.advance = face->glyph->advance.x >> 6;          // Convert to pixels

        // Update UV coordinates based on the packed position
        Texture *temp_ref = set->atlas;
        glyph.uv0.x = (float)x / temp_ref->width;
        glyph.uv0.y = (float)y / temp_ref->height;
        glyph.uv1.x = (float)(x + width) / temp_ref->width;
        glyph.uv1.y = (float)(y + height) / temp_ref->height;

        set->glyphs[glyph.id] = glyph;
    }
}
Glyph *get_glyph(Font *font, u16 id, u32 size)
{
    if (!font)
    {
        LOG_ERROR("get_glyph() -> Font is not initialized");
        return 0;
    }

    // check if the char with the same size is already loaded

    GlyphSet *set = get_glyphset(font, size);
    if (!set)
    {
        LOG_TRACE("get_glyph() -> Glyph set with size %d not found", size);
        return 0;
    }
    if (id >= set->glyph_count)
    {
        LOG_TRACE("get_glyph() -> Glyph id %d is out of bounds", id);
        return 0;
    }
    return set->glyphs[id].id == id ? &set->glyphs[id] : 0;
}

GlyphSet *create_glyphset(Font *tf, f32 size)
{
    GlyphSet new_set = {
        .id = font_index++,
        .glyph_count = 'z',
        .size = size,
        .glyphs = malloc(sizeof(Glyph) * 'z'),
        .atlas = malloc(sizeof(Texture)),
        .__internal__ = malloc(sizeof(GlyphSetInternal))};
    memset(new_set.glyphs, 0, sizeof(Glyph) * new_set.glyph_count);
    INTERNAL(new_set)->atlas.pixels = calloc(512 * 512, sizeof(u8) * 4);
    INTERNAL(new_set)->atlas.width = INTERNAL(new_set)->atlas.height = 512;

    Texture *temp_ref = new_set.atlas;
    temp_ref->is_dirty = true;
    temp_ref->Idx = new_set.id;
    temp_ref->height = 512;
    temp_ref->width = 512;
    // temp_ref->mipLevels = (u16)FLOOR(log2(MAX(width, height))) + 1;
    createTexture(0, new_set.atlas);

    DynamicArray_Push(tf->glyph_sets, new_set);
    return &tf->glyph_sets[DynamicArray_Length(tf->glyph_sets) - 1];
}

bool resize_Glyphset(GlyphSet *set)
{
    if (!set || !INTERNAL(*set)->atlas.pixels || INTERNAL(*set)->atlas.width == 0 || INTERNAL(*set)->atlas.height == 0)
    {
        LOG_ERROR("resize_Glyphset() -> Atlas not initialized");
        return false;
    }

    // Resize the atlas texture
    Texture *tex = (Texture *)set->atlas;
    tex->is_dirty = true;
    // u32 old_w = tex->width;
    tex->width *= 2;
    tex->height *= 2;

    // CPU side

    // Reinitialize the packer with the new atlas size
    if (INTERNAL(*set)->packer.nodes)
    {
        free(INTERNAL(*set)->packer.nodes);
    }
    INTERNAL(*set)->packer.node_count = tex->width / 4; // Adjust size based on atlas dimensions
    INTERNAL(*set)->packer.nodes = malloc(sizeof(stbrp_node) * INTERNAL(*set)->packer.node_count);
    if (!INTERNAL(*set)->packer.nodes)
    {
        LOG_ERROR("resize_Glyphset() -> Failed to allocate memory for packing nodes");
        return false;
    }
    stbrp_init_target(
        &INTERNAL(*set)->packer.context,
        INTERNAL(*set)->atlas.width,
        INTERNAL(*set)->atlas.height,
        INTERNAL(*set)->packer.nodes,
        INTERNAL(*set)->packer.node_count);
    // Repack the glyphs into the new atlas size
    u8 *new_pixels = calloc(tex->width * tex->height * 4, sizeof(u8));
    if (!new_pixels)
    {
        LOG_ERROR("resize_Glyphset() -> Failed to allocate memory for new atlas pixels");
        return false;
    }

    // Step 4: Repack all valid glyphs into the new atlas
    for (u32 i = 0; i < set->glyph_count; ++i)
    {
        Glyph *glyph = &set->glyphs[i];
        if (glyph->id == 0)
            continue; // Skip invalid glyphs

        u32 x, y, x0, y0;
        // u32* buffer = &INTERNAL(*set)->atlas.pixels[(((u32)glyph->uv0.y) * old_w + ((u32)glyph->uv0.x)) * 4]; // Get the pixel data from the old atlas

        // Attempt to pack the glyph into the new atlas
        stbrp_rect rect = {.id = 0, .w = glyph->width, .h = glyph->height};
        if (!stbrp_pack_rects(&INTERNAL(*set)->packer.context, &rect, 1))
        {
            LOG_ERROR("resize_Glyphset() -> Failed to repack glyph %d into new atlas", i);
            free(new_pixels);
            free(INTERNAL(*set)->packer.nodes);
            INTERNAL(*set)->packer.nodes = 0;
            return false;
        }

        // Copy the glyph bitmap into the new pixel buffer
        x = rect.x;
        y = rect.y;
        x0 = x + glyph->width;
        y0 = y + glyph->height;
        for (u32 row = 0; row < glyph->height; ++row)
        {
            for (u32 col = 0; col < glyph->width; ++col)
            {
                u32 srcIndex = (glyph->uv0.y + row) * INTERNAL(*set)->atlas.width + (glyph->uv0.x + col);
                u32 dstIndex = (y + row) * tex->width + (x + col);

                // Copy the grayscale value into the red channel (R)
                new_pixels[dstIndex * 4 + 0] = INTERNAL(*set)->atlas.pixels[srcIndex * 4 + 0];
                // Set green and blue channels to match (grayscale) & alpha channel (A)
                new_pixels[dstIndex * 4 + 1] = new_pixels[dstIndex * 4 + 2] = new_pixels[dstIndex * 4 + 3] =
                    INTERNAL(*set)->atlas.pixels[srcIndex * 4 + 0];
            }
        }

        // Update glyph metadata with the new position
        glyph->uv0.x = x / tex->width;
        glyph->uv0.y = y / tex->height;
        glyph->uv1.x = x0 / (tex->width);
        glyph->uv1.y = y0 / (tex->height);
    }
    free(INTERNAL(*set)->atlas.pixels);
    INTERNAL(*set)->atlas.pixels = new_pixels;
    INTERNAL(*set)->atlas.width = tex->width;
    INTERNAL(*set)->atlas.height = tex->height;

    // GPU side
    destroyTexture(tex);
    createTexture(0, set->atlas);
    fillTextureRegion(tex, INTERNAL(*set)->atlas.pixels, 0, 0, INTERNAL(*set)->atlas.width, INTERNAL(*set)->atlas.height);
    return true;
}

void destroy_glyphset(GlyphSet *set)
{
    if (INTERNAL(*set)->packer.nodes)
    {
        free(INTERNAL(*set)->packer.nodes);
    }
    if (INTERNAL(*set)->atlas.pixels)
    {
        free(INTERNAL(*set)->atlas.pixels);
    }
    if (set->glyphs)
    {
        free(set->glyphs);
    }
    if (set->__internal__)
    {
        free(set->__internal__);
    }
    set->glyph_count = 0;
    if (set->atlas)
    {
        destroyTexture((Texture *)set->atlas);
        free(set->atlas);
        set->atlas = 0;
    }
}

bool packGlyphIntoAtlas(GlyphSet *glyphSet, FT_Bitmap *bitmap, u32 *outX, u32 *outY)
{
    GlyphSetInternal *__internal__ = INTERNAL(*glyphSet);
    // initializing the packer if not initialised
    if (__internal__->packer.nodes == 0)
    {
        __internal__->packer.node_count = __internal__->atlas.width / 4; // Adjust size based on atlas dimensions
        __internal__->packer.nodes = malloc(sizeof(stbrp_node) * __internal__->packer.node_count);
        if (!__internal__->packer.nodes)
        {
            LOG_ERROR("Failed to allocate memory for packing nodes");
            return false;
        }

        stbrp_init_target(
            &__internal__->packer.context,
            __internal__->atlas.width,
            __internal__->atlas.height,
            __internal__->packer.nodes,
            __internal__->packer.node_count);
    }

    // Ensure the atlas has enough space for the glyph
    if (!glyphSet || !__internal__->atlas.pixels || __internal__->atlas.width == 0 || __internal__->atlas.height == 0)
    {
        LOG_ERROR("packGlyphIntoAtlas() -> Atlas not initialized");
        return false;
    }

    // Define the rectangle for the glyph
    const int MAX_RECTS = 1;
    stbrp_rect rects[MAX_RECTS];
    rects[0].id = 0;
    rects[0].w = bitmap->width;
    rects[0].h = bitmap->rows;

    // Attempt to pack the rectangle into the atlas using the persistent context
    if (!stbrp_pack_rects(&__internal__->packer.context, rects, MAX_RECTS))
    {
        if (!resize_Glyphset(glyphSet))
        {
            LOG_ERROR("packGlyphIntoAtlas() -> Failed to resize glyphset atlas");
            return false;
        }
        if (!stbrp_pack_rects(&__internal__->packer.context, rects, MAX_RECTS))
        {
            LOG_ERROR("packGlyphIntoAtlas() -> Failed to pack glyph into atlas even after resizing");
            return false;
        }
    }

    // Retrieve the packed position
    *outX = rects[0].x;
    *outY = rects[0].y;

    u8 *tempPixels = malloc(bitmap->width * bitmap->rows * 4); // RGBA format
    if (!tempPixels)
    {
        LOG_ERROR("packGlyphIntoAtlas() -> Failed to allocate memory for temporary glyph pixels");
        return false;
    }

    // Copy the glyph bitmap into the atlas image buffer
    u8 *pixels_ref = __internal__->atlas.pixels;
    for (u32 y = 0; y < bitmap->rows; ++y)
    {
        for (u32 x = 0; x < bitmap->width; ++x)
        {
            u32 srcIndex = y * bitmap->width + x;                                       // Source index in the glyph bitmap
            u32 dstIndex = ((*outY + y) * __internal__->atlas.width + (*outX + x)) * 4; // Destination index in the atlas
            u32 tempIndex = (y * bitmap->width + x) * 4;                                // Destination index in the temporary buffer

            // Copy the grayscale value into the red channel (R)
            pixels_ref[dstIndex + 0] = bitmap->buffer[srcIndex];
            // Set green and blue channels to match (grayscale) & and alpha channel (A)
            pixels_ref[dstIndex + 1] = pixels_ref[dstIndex + 2] = pixels_ref[dstIndex + 3] = bitmap->buffer[srcIndex];

            // Copy the same data into the temporary buffer
            tempPixels[tempIndex + 0] = bitmap->buffer[srcIndex];
            tempPixels[tempIndex + 1] = tempPixels[tempIndex + 2] = tempPixels[tempIndex + 3] = bitmap->buffer[srcIndex];
        }
    }
    // fillTextureRegion(
    //     glyphSet->atlas,
    //     &__internal__->atlas.pixels[*outY * __internal__->atlas.width + *outX],
    //     *outX, *outY, bitmap->width, bitmap->rows);
    fillTextureRegion(
        glyphSet->atlas,
        // &__internal__->atlas.pixels[((*outY * __internal__->atlas.width) + *outX) * 4],
        tempPixels,
        *outX, *outY, bitmap->width, bitmap->rows);
    ((Texture *)glyphSet->atlas)->is_dirty = true;
    return true;
}