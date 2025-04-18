#include "assets/loaders/font_loader.h"

#include "core/files.h"
#include "core/debugger.h"
#include <collections/DynamicArray.h>
#include <math/vec2.h>
#include "renderer/render_types.h"

#include "assets/asset_manager.h"

#include <stdlib.h>
#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H

Asset load_font_bitmap(const char* path, const char* name);
Asset load_font_ttf(const char* path);
Glyph* get_glyph(Font* font, u16 id, u32 size);

static FT_Library* library = 0; //TODO: free this when shutting down the asset manager | move managing it to loader inti & shutdown

void init_font_loader(){

}

void shutdown_font_loader(){
    if(library){
        free(library);
        library = 0;
    }
}

Asset load_font(const char* path, void* name_ptr){
    String ext = path_get_file_extension(path);
    if(str_equals_val(ext, "fnt")){
        return load_font_bitmap(path, (char*)name_ptr);
    }else if(str_equals_val(ext, "ttf")){
        return load_font_ttf(path);
    }else{
        LOG_ERROR("load_font() -> Unsupported font format: %s", ext.val);
        return (Asset){0};
    }
}

/// @brief 
/// @param path 
/// @param name 
/// @return 
Asset load_font_bitmap(const char* path, const char* name){
    FileHandle* fntFile = file_load(path);
    if(!fntFile){
        LOG_ERROR("load_font() -> file does not exist or could not be loaded.");
        return NULL_ASSET;
    }

    String pngFile = str_join(str_join(path_get_file_dir(path), path_get_file_name(path)), str_new(".png"));
    Asset *atlas = load_asset((const char *)pngFile.val, str_join(str_new(name), str_new("_atlas")).val);
    Vec2 atlas_size = vec2_new(((Texture *)atlas->data)->width, ((Texture *)atlas->data)->height);

    if (!atlas)
    {
        LOG_ERROR("load_asset() : failed to load font atlas %s", pngFile.val);
        return NULL_ASSET;
    }
    
    Asset* font_asset = get_asset(name, ASSET_TYPE_FONT);
    Font* font;
    if(!font_asset){
        font = malloc(sizeof(Font));
        font->glyph_sets = DynamicArray_Create(GlyphSet);
    }else{
        font = (Font*)font_asset->data;
    }
    
    Glyph* glyphs = DynamicArray_Create(Glyph);
    Glyph* glyph = malloc(sizeof(Glyph));
    f32 font_size = 0;
    for(u32 i=0; i < fntFile->line_count; i++, file_next_line(fntFile)){
        String line = file_line_toString(fntFile);
        if(line.val[0] == 'c' && line.val[1] == 'o'){
            //parse common
        }else if(line.val[0] == 'i' && line.val[1] == 'n'){
            //parse info
            sscanf_s(line.val, "info%*[^s]size=%f", &font_size);
        }else if(line.val[0] == 'p' && line.val[1] == 'a'){
            //parse page
        }else if(line.val[0] == 'c' && line.val[1] == 'h' && line.val[4] == 's'){
            //parse chars
        }else if(line.val[0] == 'c' && line.val[1] == 'h' && line.val[4] == ' '){
            //parse char
            *glyph = (Glyph){0};
            u32 page, chnl;
            sscanf_s(line.val, "char id=%hd x=%f y=%f width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d",
                &glyph->id, &glyph->uv0.x, &glyph->uv0.y, &glyph->width, &glyph->height, &glyph->offset.x, &glyph->offset.y, &glyph->advance, &page, &chnl);
            glyph->uv1.x = (glyph->uv0.x + (f32)glyph->width)/atlas_size.x;
            glyph->uv1.y = (glyph->uv0.y + (f32)glyph->height)/atlas_size.y;
            glyph->uv0.x /= atlas_size.x;
            glyph->uv0.y /= atlas_size.y;
            DynamicArray_Push(glyphs, *glyph);
        }else if(line.val[0] == 'k' && line.val[1] == 'e'){
            //parse kerning
            //TODO: add kerning support
        }
    }

    GlyphSet _temp = {};
    _temp.size = font_size;
    _temp.atlas = atlas->data;
    const u32 glyphsCount = DynamicArray_Length(glyphs);
    _temp.glyph_count = glyphsCount;
    //find the max glyph id
    for(u32 i=0; i< glyphsCount; i++){
        if(glyphs[i].id >= _temp.glyph_count){
            _temp.glyph_count = glyphs[i].id+1;
        }
    }
    //copy glyphs to a semi packed array
    _temp.glyphs = malloc(sizeof(Glyph) * _temp.glyph_count);
    for(u32 i=0; i < _temp.glyph_count; i++){
        if(glyphs[i].id >= _temp.glyph_count){
            LOG_ERROR("load_font() -> glyph id %d is out of bounds", glyphs[i].id);
            continue;
        }
        _temp.glyphs[glyphs[i].id] = glyphs[i];
    }
    DynamicArray_Destroy(glyphs);

    DynamicArray_Push(font->glyph_sets, _temp);

    Asset new = {0};
    new.data = font;
    new.ref_count = 0;
    new.type = ASSET_TYPE_FONT;
    new.name = str_new(path);
    return new;   
}


Asset load_font_ttf(const char* path){
    FT_Error error;
    if(!library){
        library = malloc(sizeof(FT_Library));
        error = FT_Init_FreeType(library); 
        if (error) {
            LOG_ERROR("load_font_ttf() -> Could not initialize FreeType library");
            return NULL_ASSET;
        }
    }

    Font* font = malloc(sizeof(Font));
    error = FT_New_Face(*library, path, 0, &font->face);
    if (error == FT_Err_Unknown_File_Format) {
        LOG_ERROR("load_font_ttf() -> Unsupported font format: %s", path_get_file_extension(path).val);
        FT_Done_FreeType(*library);
        return NULL_ASSET;
    }else if (error == FT_Err_Cannot_Open_Resource) {
        LOG_ERROR("load_font_ttf() -> Could not open font file: %s", path);
        FT_Done_FreeType(*library);
        return NULL_ASSET;
    }else if (error) {
        LOG_ERROR("load_font_ttf() -> Could not load font: %s", path);
        FT_Done_FreeType(*library);
        return NULL_ASSET;
    }


    Asset new = {0};
    new.data = 0;
    new.ref_count = 0;
    new.type = ASSET_TYPE_FONT;
    new.name = str_new(path);
    return new;   
}

void release_font(Asset* font){
    for(u32 i=0; i< DynamicArray_Length(((Font*)font->data)->glyph_sets); i++){
        GlyphSet set = ((Font*)font->data)->glyph_sets[i];
        if(set.glyphs){
            free(set.glyphs);
        }
    }
    free((Font*)font->data);
}

GlyphSet* get_glyphset(Font* font, u32 size){
    if(!font){
        LOG_ERROR("get_glyphset() -> Font is not initialized");
        return 0;
    }

    //check if the char with the same size is already loaded
    for(u32 i=0; i< DynamicArray_Length(font->glyph_sets); i++){
        GlyphSet set = font->glyph_sets[i];
        if(set.size == size){
            return &font->glyph_sets[i];
        }
    }
    return 0;
}

void load_char(Font* tf, char c, u32 size){
    if(!tf){
        LOG_ERROR("load_char() -> Font is not initialized");
        return;
    }

    //check if the char with the same size is already loaded
    Glyph* ch = get_glyph(tf, c, size);
    if(ch){
        return;
    }
    //if font is not a ttf return since fnt fonts load all chars at once
    if(!tf->face){
        LOG_WARN("load_char() -> Font is not a ttf font, cannot load char %c", c);
        return;
    }

    FT_Face face = tf->face;
    FT_Set_Pixel_Sizes(face, 0, size);
    FT_Bitmap bitmap = face->glyph->bitmap;
    uint8_t* pixels = bitmap.buffer;
    int width = bitmap.width;
    int height = bitmap.rows;

    //get the font atlas asset
    //update the font atlas texture with the new glyph
    //update the font glyph data with the new glyph
}

Glyph* get_glyph(Font* font, u16 id, u32 size){
    if(!font){
        LOG_ERROR("get_glyph() -> Font is not initialized");
        return 0;
    }

    //check if the char with the same size is already loaded

    GlyphSet* set = get_glyphset(font, size);
    if(!set){
        LOG_ERROR("get_glyph() -> Glyph set with size %d not found", size);
        return 0;
    }
    if(id >= set->glyph_count){
        LOG_ERROR("get_glyph() -> Glyph id %d is out of bounds", id);
        return 0;
    }
    return set->glyphs[id].id == id ? &set->glyphs[id] : 0;
}