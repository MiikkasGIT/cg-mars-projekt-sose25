#include "Terrain.h"
#include "TerrainShader.h"
#include "rgbimage.h"
#include <cstdlib>

template <typename T>
static inline T clampv(T v, T lo, T hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

Terrain::Terrain(const char* DetailMap1, const char* DetailMap2)
{
    bool ok = true;
    if(DetailMap1) ok &= DetailTex[0].load(DetailMap1);
    if(DetailMap2) ok &= DetailTex[1].load(DetailMap2);
    if(!ok) throw std::exception();
}

Terrain::~Terrain() {}

bool Terrain::loadDetailMix(const char* DetailMap1, const char* DetailMap2, const char* MixMap)
{
    bool ok = true;
    if(DetailMap1) ok &= DetailTex[0].load(DetailMap1);
    if(DetailMap2) ok &= DetailTex[1].load(DetailMap2);
    if(MixMap)     ok &= MixTex.load(MixMap);
    return ok;
}

Vector Terrain::normalCalc(const Vector& p1, const Vector& p2, const Vector& p3) const
{
    return (p2 - p1).cross(p3 - p1);
}

// ------------------- Diamond–Square API -------------------

bool Terrain::generateDiamondSquare(int size, float roughness, unsigned int seed,
                                    float worldScale, float heightScale, bool wrapEdges)
{
    int n = size - 1;
    if (size < 3 || (n & (n - 1)) != 0) {
        return false; // size muss 2^k + 1 sein
    }

    std::vector<float> h(size * size, 0.0f);

    // 1. Ecken initialisieren (0..1)
    h[idx(0, 0, size)] = 0.5f;
    h[idx(n, 0, size)] = 0.5f;
    h[idx(0, n, size)] = 0.5f;
    h[idx(n, n, size)] = 0.5f;

    int   step      = n;
    float amplitude = 0.5f;
    const float decay = clampv(roughness, 0.01f, 0.99f);

    auto frand = [&](float a, int x, int z){ // deterministisch aus (x,z,seed)
        return (hashNoise(x, z, seed) * 2.0f - 1.0f) * a;
    };

    // 2. Diamond-Square Iterationen
    while (step > 1) {
        int half = step / 2;

        // Diamond Step
        for (int z = 0; z < n; z += step) {
            for (int x = 0; x < n; x += step) {
                float c00 = h[idx(x,       z,       size)];
                float c10 = h[idx(x+step,  z,       size)];
                float c01 = h[idx(x,       z+step,  size)];
                float c11 = h[idx(x+step,  z+step,  size)];
                float avg = 0.25f * (c00 + c10 + c01 + c11);

                int cx = x + half, cz = z + half;
                if (wrapEdges) { cx %= size; cz %= size; }
                h[idx(cx, cz, size)] = clampv(avg + frand(amplitude, cx, cz), 0.0f, 1.0f);
            }
        }

        // Square Step
        for (int z = 0; z <= n; z += half) {
            int startX = ((z/half) % 2 == 0) ? half : 0;
            for (int x = startX; x <= n; x += step) {
                dsDiamondStep(h, size, x, z, half, amplitude, wrapEdges);
            }
        }

        step      /= 2;
        amplitude *= decay;
    }

    // 3. Normalisieren (0..1)
    float mn = h[0], mx = h[0];
    for (float v : h) { if (v < mn) mn = v; if (v > mx) mx = v; }
    float range = (mx > mn) ? (mx - mn) : 1.0f;
    for (float& v : h) v = (v - mn) / range;

    // 4. Mesh bauen
    buildMeshFromHeights(h, size, size, worldScale, heightScale);
    return true;
}

void Terrain::dsDiamondStep(std::vector<float>& h, int size, int x, int z, int reach,
                            float amplitude, bool wrap)
{
    auto inside = [&](int xx, int zz)->bool {
        return (xx >= 0 && xx < size && zz >= 0 && zz < size);
    };

    float sum = 0.0f; int cnt = 0;

    // vier Nachbarn sammeln (ohne NaN/limits)
    int px = x - reach, nx = x + reach, pz = z - reach, nz = z + reach;

    auto sample = [&](int sx, int sz)->float {
        if (wrap) {
            int wx = (sx % size + size) % size;
            int wz = (sz % size + size) % size;
            return h[idx(wx, wz, size)];
        } else {
            if (!inside(sx, sz)) return 0.0f;
            return h[idx(sx, sz, size)];
        }
    };

    if (wrap || inside(px, z)) { sum += sample(px, z); ++cnt; }
    if (wrap || inside(nx, z)) { sum += sample(nx, z); ++cnt; }
    if (wrap || inside(x, pz)) { sum += sample(x, pz); ++cnt; }
    if (wrap || inside(x, nz)) { sum += sample(x, nz); ++cnt; }

    if (cnt == 0) return;

    float avg = sum / cnt;
    float jitter = hashNoise(x, z, 1337u) * 2.0f - 1.0f; // deterministisch
    float val = clampv(avg + jitter * amplitude, 0.0f, 1.0f);

    int sx = wrap ? (x % size + size) % size : x;
    int sz = wrap ? (z % size + size) % size : z;
    if (inside(sx, sz)) h[idx(sx, sz, size)] = val;
}

// kleine deterministische Pseudozufallsfunktion (ohne <random>)
inline float Terrain::hashNoise(int x, int z, unsigned int seed) const
{
    // 2D-Hash -> [0,1]
    unsigned int h = seed;
    h ^= 374761393u + (unsigned int)x * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    h ^= 2246822519u + (unsigned int)z * 3266489917u;
    h = (h ^ (h >> 13)) * 2246822519u;
    h ^= h >> 16;
    // normieren
    return (h & 0xFFFFFFu) / float(0x1000000u);
}

// ------------------- Heightmap laden -------------------

bool Terrain::load(const char* HeightMap, const char* DetailMap1, const char* DetailMap2, const char* MixMap)
{
    if (!HeightTex.load(HeightMap)) return false;
    if (DetailMap1 && !DetailTex[0].load(DetailMap1)) return false;
    if (DetailMap2 && !DetailTex[1].load(DetailMap2)) return false;
    if (MixMap     && !MixTex.load(MixMap))           return false;

    const RGBImage* image = HeightTex.getRGBImage();
    if(!image) return false;

    const int imgWidth  = image->width();
    const int imgHeight = image->height();

    std::vector<float> heights(imgWidth * imgHeight, 0.0f);
    for (int z = 0; z < imgHeight; ++z) {
        for (int x = 0; x < imgWidth; ++x) {
            Color c = image->getPixelColor(x, z);
            float g = (c.R + c.G + c.B) / 3.0f;      // 0..1
            float h = 1.0f - clampv(g, 0.0f, 1.0f);  // invertiert wie zuvor
            heights[idx(x, z, imgWidth)] = h;
        }
    }

    // typische Skalen
    float worldScale  = 1.0f;
    float heightScale = 1.0f;

    buildMeshFromHeights(heights, imgWidth, imgHeight, worldScale, heightScale);
    return true;
}

// ------------------- Gemeinsamer Mesh-Builder -------------------

void Terrain::buildMeshFromHeights(const std::vector<float>& heights, int width, int height,
                                   float worldScale, float heightScale)
{
    GridW = width;
    GridH = height;
    WorldScale  = worldScale;
    HeightScale = heightScale;
    Heights = heights;

    std::vector<Vector> flatVertices(width * height);
    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            float posX = x * worldScale;
            float posZ = z * worldScale;
            float posY = heights[idx(x, z, width)] * heightScale;
            flatVertices[idx(x, z, width)] = Vector(posX, posY, posZ);
        }
    }

    std::vector<Vector> vertexNormals(width * height, Vector(0,0,0));
    for (int z = 0; z < height - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            int tl = idx(x,   z,   width);
            int tr = idx(x+1, z,   width);
            int bl = idx(x,   z+1, width);
            int br = idx(x+1, z+1, width);

            const Vector& v0 = flatVertices[tl];
            const Vector& v1 = flatVertices[bl];
            const Vector& v2 = flatVertices[tr];
            Vector n1 = (v1 - v0).cross(v2 - v0);

            vertexNormals[tl] += n1;
            vertexNormals[bl] += n1;
            vertexNormals[tr] += n1;

            const Vector& v3 = flatVertices[tr];
            const Vector& v4 = flatVertices[bl];
            const Vector& v5 = flatVertices[br];
            Vector n2 = (v4 - v3).cross(v5 - v3);

            vertexNormals[tr] += n2;
            vertexNormals[bl] += n2;
            vertexNormals[br] += n2;
        }
    }

    // einfache Glättung (3x3 Box)
    std::vector<Vector> smoothed(width * height, Vector(0,0,0));
    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            Vector acc = vertexNormals[idx(x, z, width)];
            for (int oz = -1; oz <= 1; ++oz)
                for (int ox = -1; ox <= 1; ++ox) {
                    int nx = x + ox, nz = z + oz;
                    if (nx >= 0 && nx < width && nz >= 0 && nz < height)
                        acc += vertexNormals[idx(nx, nz, width)];
                }
            smoothed[idx(x, z, width)] = acc.normalize();
        }
    }

    // Vertex Buffer
    VB.begin();
    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            int i = idx(x, z, width);
            VB.addNormal(smoothed[i]);
            VB.addTexcoord0(static_cast<float>(x) / (width  - 1),
                            static_cast<float>(z) / (height - 1));
            VB.addTexcoord1((static_cast<float>(x) / (width  - 1)) * 100.0f,
                            (static_cast<float>(z) / (height - 1)) * 100.0f);
            VB.addVertex(flatVertices[i]);
        }
    }
    VB.end();

    // Index Buffer
    IB.begin();
    for (int z = 0; z < height - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            int i = idx(x, z, width);
            IB.addIndex(i);
            IB.addIndex(i + width + 1);
            IB.addIndex(i + 1);

            IB.addIndex(i);
            IB.addIndex(i + width);
            IB.addIndex(i + width + 1);
        }
    }
    IB.end();
}

// ------------------- Render/Shader -------------------

void Terrain::shader(BaseShader* shader, bool deleteOnDestruction)
{
    BaseModel::shader(shader, deleteOnDestruction);
}

void Terrain::draw(const BaseCamera& Cam)
{
    applyShaderParameter();
    BaseModel::draw(Cam);

    VB.activate();
    IB.activate();
    glDrawElements(GL_TRIANGLES, IB.indexCount(), IB.indexFormat(), 0);
    IB.deactivate();
    VB.deactivate();
}

void Terrain::applyShaderParameter()
{
    TerrainShader* Shader = dynamic_cast<TerrainShader*>(BaseModel::shader());
    if(!Shader) return;

    Shader->mixTex(&MixTex);
    for(int i=0; i<2; i++)
        Shader->detailTex(i,&DetailTex[i]);
    Shader->scaling(Size);
}

// ------------------- Height Sampling -------------------

float Terrain::sampleHeightLocal(float lx, float lz) const
{
    if (GridW <= 1 || GridH <= 1 || Heights.empty())
        return 0.0f;

    float gx = lx / WorldScale;
    float gz = lz / WorldScale;

    // clamp in gültigen Bereich
    gx = clampv(gx, 0.0f, float(GridW - 1));
    gz = clampv(gz, 0.0f, float(GridH - 1));

    int x0 = int(std::floor(gx));
    int z0 = int(std::floor(gz));
    int x1 = (x0 + 1 < GridW) ? x0 + 1 : x0;
    int z1 = (z0 + 1 < GridH) ? z0 + 1 : z0;

    float tx = gx - float(x0);
    float tz = gz - float(z0);

    float h00 = Heights[idx(x0, z0, GridW)];
    float h10 = Heights[idx(x1, z0, GridW)];
    float h01 = Heights[idx(x0, z1, GridW)];
    float h11 = Heights[idx(x1, z1, GridW)];

    float h0 = h00 * (1.0f - tx) + h10 * tx;
    float h1 = h01 * (1.0f - tx) + h11 * tx;
    float hN = h0  * (1.0f - tz) + h1  * tz;

    return hN * HeightScale; // Welt-Y
}

float Terrain::heightAtWorld(float xw, float zw) const
{
    // Welt -> Objektraum (inverse Model-Transform)
    Matrix inv = transform();
    inv.invert();
    Vector local = inv * Vector(xw, 0.0f, zw);
    return sampleHeightLocal(local.X, local.Z);
}

float Terrain::distanceToTerrain(const Vector& worldPos) const
{
    float groundY = heightAtWorld(worldPos.X, worldPos.Z);
    return worldPos.Y - groundY;
}
