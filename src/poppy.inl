static unsigned long long ConstantTextureLookupHash(const char* str);
ConstantTextureLookup ConstantTextureLookupCreate(size_t capacity);
void ConstantTextureLookupAdd(ConstantTextureLookup* lookup, const char* key, ConstantTexture* texture);


