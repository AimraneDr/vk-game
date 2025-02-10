#include "assets/details/model_loader.h"
#include "math/mathTypes.h"
#include "core/files.h"
#include "core/debugger.h"
#include <stdlib.h>
#include <string.h>
#include <string/str.h>
#include <math/mathUtils.h>

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <TinyObjLoader/tiny_obj_loader_c.h>

// Hash function for vertex data
static u32 hash_vertex(const Vertex* vertex) {
    u32 hash = 0;
    const float* data = (const float*)vertex;
    for (int i = 0; i < sizeof(Vertex) / sizeof(float); i++) {
        hash = hash * 31 + (u32)(data[i] * 1000000.0f); // Scale floats to reduce precision errors
    }
    return hash;
}

// Compare two vertices for equality
static bool vertices_equal(const Vertex* a, const Vertex* b) {
    const float epsilon = 0.000001f; // Small epsilon for float comparison
    return ABS(a->pos.x - b->pos.x) < epsilon &&
           ABS(a->pos.y - b->pos.y) < epsilon &&
           ABS(a->pos.z - b->pos.z) < epsilon &&
           ABS(a->norm.x - b->norm.x) < epsilon &&
           ABS(a->norm.y - b->norm.y) < epsilon &&
           ABS(a->norm.z - b->norm.z) < epsilon &&
           ABS(a->texCoord.x - b->texCoord.x) < epsilon &&
           ABS(a->texCoord.y - b->texCoord.y) < epsilon;
}

// Hashmap entry for vertex deduplication
typedef struct {
    Vertex vertex;
    u32 index;
    struct HashMapEntry* next;
} HashMapEntry;

// Hashmap structure
typedef struct {
    HashMapEntry** buckets;
    size_t bucket_count;
} HashMap;

// Initialize hashmap
static HashMap* hashmap_create(size_t bucket_count) {
    HashMap* map = malloc(sizeof(HashMap));
    map->buckets = calloc(bucket_count, sizeof(HashMapEntry*));
    map->bucket_count = bucket_count;
    return map;
}

// Free hashmap and all entries
static void hashmap_destroy(HashMap* map) {
    for (size_t i = 0; i < map->bucket_count; i++) {
        HashMapEntry* entry = map->buckets[i];
        while (entry) {
            HashMapEntry* next = entry->next;
            free(entry);
            entry = next;
        }
    }
    free(map->buckets);
    free(map);
}

// Find or insert vertex in hashmap
static u32 hashmap_find_or_insert(HashMap* map, const Vertex* vertex, u32 next_index) {
    u32 hash = hash_vertex(vertex);
    size_t bucket = hash % map->bucket_count;
    
    // Check for existing vertex
    HashMapEntry* entry = map->buckets[bucket];
    while (entry) {
        if (vertices_equal(&entry->vertex, vertex)) {
            return entry->index;
        }
        entry = entry->next;
    }
    
    // Insert new vertex
    entry = malloc(sizeof(HashMapEntry));
    entry->vertex = *vertex;
    entry->index = next_index;
    entry->next = map->buckets[bucket];
    map->buckets[bucket] = entry;
    
    return next_index;
}


// Callback to read file contents into buf
static void obj_file_reader(void* ctx, const char* filename, int is_mtl, 
                          const char* obj_filename, char** buf, size_t* len) {
    File* file = readFile(filename);
    if (!file || !file->content) {
        *buf = NULL;
        *len = 0;
        return;
    }
    *buf = file->content;
    *len = file->size;
    free(file->path);
    free(file);
}

Asset load_obj(const char* path) {
    Asset asset = {0};
    asset.type = ASSET_TYPE_MODEL;
    asset.name = str_new(path);

    tinyobj_attrib_t attrib;
    tinyobj_attrib_init(&attrib);
    tinyobj_shape_t* shapes = NULL;
    size_t num_shapes = 0;
    tinyobj_material_t* materials = NULL;
    size_t num_materials = 0;

    int ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes,
                               &materials, &num_materials,
                               path, obj_file_reader, NULL,
                               TINYOBJ_FLAG_TRIANGULATE);

    if (ret != TINYOBJ_SUCCESS) {
        LOG_ERROR("Error: Failed to load OBJ file %s", path);
        asset.data = NULL;
        return asset;
    }

    // Create temporary storage for unique vertices
    HashMap* vertex_map = hashmap_create(attrib.num_faces);
    Vertex* unique_vertices = malloc(sizeof(Vertex) * attrib.num_faces);
    u32* indices = malloc(sizeof(u32) * attrib.num_faces);
    size_t unique_vertex_count = 0;

    // Process all faces and deduplicate vertices
    for (size_t i = 0; i < attrib.num_faces; i++) {
        tinyobj_vertex_index_t idx = attrib.faces[i];
        Vertex vertex = {0};

        // Extract position
        if (idx.v_idx >= 0) {
            vertex.pos.x = attrib.vertices[3 * idx.v_idx + 0];
            vertex.pos.y = attrib.vertices[3 * idx.v_idx + 1];
            vertex.pos.z = attrib.vertices[3 * idx.v_idx + 2];
        }

        // Extract normal
        if (idx.vn_idx >= 0 && attrib.normals) {
            vertex.norm.x = attrib.normals[3 * idx.vn_idx + 0];
            vertex.norm.y = attrib.normals[3 * idx.vn_idx + 1];
            vertex.norm.z = attrib.normals[3 * idx.vn_idx + 2];
        }

        // Extract texture coordinates
        if (idx.vt_idx >= 0 && attrib.texcoords) {
            vertex.texCoord.x = attrib.texcoords[2 * idx.vt_idx + 0];
            vertex.texCoord.y = 1.f - attrib.texcoords[2 * idx.vt_idx + 1];
        }

        // Find or add vertex to unique set
        u32 vertex_idx = hashmap_find_or_insert(vertex_map, &vertex, unique_vertex_count);
        if (vertex_idx == unique_vertex_count) {
            unique_vertices[unique_vertex_count] = vertex;
            unique_vertex_count++;
        }
        indices[i] = vertex_idx;
    }

    // Create final model with deduplicated vertices
    Model* model = malloc(sizeof(Model));
    if (!model) {
        LOG_ERROR("Error: Memory allocation failed for Model");
        free(unique_vertices);
        free(indices);
        hashmap_destroy(vertex_map);
        asset.data = NULL;
        return asset;
    }

    model->id = 0;
    model->vertex_count = unique_vertex_count;
    model->index_count = attrib.num_faces;
    model->vertices = malloc(sizeof(Vertex) * unique_vertex_count);
    model->indices = malloc(sizeof(u32) * attrib.num_faces);

    if (!model->vertices || !model->indices) {
        LOG_ERROR("Error: Memory allocation failed for vertices/indices");
        free(model->vertices);
        free(model->indices);
        free(model);
        free(unique_vertices);
        free(indices);
        hashmap_destroy(vertex_map);
        asset.data = NULL;
        return asset;
    }

    // Copy deduplicated data to final model
    memcpy(model->vertices, unique_vertices, sizeof(Vertex) * unique_vertex_count);
    memcpy(model->indices, indices, sizeof(u32) * attrib.num_faces);

    // Clean up temporary storage
    free(unique_vertices);
    free(indices);
    hashmap_destroy(vertex_map);

    // Free tinyobj memory
    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);

    asset.data = model;
    return asset;
}

void release_obj(Asset* asset) {
    if (!asset || asset->type != ASSET_TYPE_MODEL || !asset->data) return;
    
    Model* model = (Model*)asset->data;
    free(model->vertices);
    free(model->indices);
    free(model);
    
    asset->data = NULL;
    str_free(&asset->name);
}