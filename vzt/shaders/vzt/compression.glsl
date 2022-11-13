// Use oct encode from: A Survey of Efficient Representations for Independent Unit Vectors [Cigolle2014]
vec2 signNotZero(vec2 v) { return vec2((v.x >= 0.0) ? +1.0 : -1.0, (v.y >= 0.0) ? +1.0 : -1.0); }
vec2 encodeNormal(in vec3 v)
{
    float l1norm = abs(v.x) + abs(v.y) + abs(v.z);
    vec2  result = v.xz * (1.0 / l1norm);
    if (v.y < 0.0)
    {
        result = (1.0 - abs(result.yx)) * signNotZero(result.xy);
    }
    return result;
}

vec3 decodeNormal(const vec2 e)
{
    vec3 v = vec3(e.x, 1.0 - abs(e.x) - abs(e.y), e.y);
    if (v.y < 0)
    {
        v.xz = (1.0 - abs(v.zx)) * signNotZero(v.xz);
    }
    return normalize(v);
}
