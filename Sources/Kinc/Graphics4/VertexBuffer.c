#include "pch.h"

#include "VertexBuffer.h"

void Kinc_G4_VertexElement_Create(Kinc_G4_VertexElement *element, const char *name, Kinc_G4_VertexData data) {
	element->name = name;
	element->data = data;
}

void Kinc_G4_VertexStructure_Create(Kinc_G4_VertexStructure *structure) {
	structure->size = 0;
	structure->instanced = false;
}

void Kinc_G4_VertexStructure_Add(Kinc_G4_VertexStructure *structure, const char *name, Kinc_G4_VertexData data) {
	Kinc_G4_VertexElement_Create(&structure->elements[structure->size++], name, data);
}
