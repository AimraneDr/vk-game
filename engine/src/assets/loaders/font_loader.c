#include "assets/loaders/font_loader.h"

#include "core/files.h"
#include "core/debugger.h"
#include <collections/DynamicArray.h>


#include <stdlib.h>
#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H

Asset load_font_bitmap(const char* path, Vec2 atlas_size, void* atlas){
    FileHandle* fntFile = file_load(path);
    if(!fntFile){
        LOG_ERROR("load_font() -> file does not exist or could not be loaded.");
        return (Asset){0};
    }
    
    Font* font = malloc(sizeof(Font));
    Glyph* glyphs = DynamicArray_Create(Glyph);
    Glyph* glyph = malloc(sizeof(Glyph));
    for(u32 i=0; i < fntFile->line_count; i++, file_next_line(fntFile)){
        String line = file_line_toString(fntFile);
        if(line.val[0] == 'c' && line.val[1] == 'o'){
            //parse common
        }else if(line.val[0] == 'i' && line.val[1] == 'n'){
            //parse info
            sscanf(line.val, "info%*[^s]size=%f", &font->size);
        }else if(line.val[0] == 'p' && line.val[1] == 'a'){
            //parse page
        }else if(line.val[0] == 'c' && line.val[1] == 'h' && line.val[4] == 's'){
            //parse chars
        }else if(line.val[0] == 'c' && line.val[1] == 'h' && line.val[4] == ' '){
            //parse char
            *glyph = (Glyph){0};
            u32 page, chnl;
            sscanf(line.val, "char id=%hd x=%f y=%f width=%d height=%d xoffset=%d yoffset=%d xadvance=%d page=%d chnl=%d",
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

    font->atlas = atlas;
    const u32 glyphsCount = DynamicArray_Length(glyphs);
    font->glyph_count = glyphsCount;
    //find the max glyph id
    for(u32 i=0; i< glyphsCount; i++){
        if(glyphs[i].id >= font->glyph_count){
            font->glyph_count = glyphs[i].id+1;
        }
    }
    //copy glyphs to a semi packed array
    font->glyphs = malloc(sizeof(Glyph) * font->glyph_count);
    for(u32 i=0; i < font->glyph_count; i++){
        if(glyphs[i].id >= font->glyph_count){
            LOG_ERROR("load_font() -> glyph id %d is out of bounds", glyphs[i].id);
            continue;
        }
        font->glyphs[glyphs[i].id] = glyphs[i];
    }
    DynamicArray_Destroy(glyphs);

    Asset new = {0};
    new.data = font;
    new.ref_count = 0;
    new.type = ASSET_TYPE_FONT;
    new.name = str_new(path);
    return new;   
}

Asset load_font_ttf(const char* path, Vec2 atlas_size, void* atlas){
    FT_Library library;
    FT_Error error;
    error = FT_Init_FreeType(&library);
    if (error) {
        LOG_ERROR("load_font_ttf() -> Could not initialize FreeType library");
        return (Asset){0};
    }

    FT_Face face;
    error = FT_New_Face(library, path, 0, &face);
    if (error == FT_Err_Unknown_File_Format) {
        LOG_ERROR("load_font_ttf() -> Unsupported font format: %s", path_get_file_extension(path).val);
        FT_Done_FreeType(library);
        return (Asset){0};
    }else if (error == FT_Err_Cannot_Open_Resource) {
        LOG_ERROR("load_font_ttf() -> Could not open font file: %s", path);
        FT_Done_FreeType(library);
        return (Asset){0};
    }else if (error) {
        LOG_ERROR("load_font_ttf() -> Could not load font: %s", path);
        FT_Done_FreeType(library);
        return (Asset){0};
    }


    Asset new = {0};
    new.data = 0;
    new.ref_count = 0;
    new.type = ASSET_TYPE_FONT;
    new.name = str_new(path);
    return new;   
}

void release_font(Asset* font){
    free(((Font*)font->data)->glyphs);
    free((Font*)font->data);
}