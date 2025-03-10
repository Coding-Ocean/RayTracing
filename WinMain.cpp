#pragma comment(lib,"winmm.lib")

#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <random>

using std::sqrt;
using std::shared_ptr;
using std::make_shared;

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// ユーティリティ関数

inline double degrees_to_radians(double degrees) {
	return degrees * pi / 180;
}

inline double random_double() {
	// [0,1) の実数乱数を返す
	return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
	// [min,max) の実数乱数を返す
	return min + (max - min) * random_double();
}

inline double random_double_cpp() {
	static std::uniform_real_distribution<double> distribution(0.0, 1.0);
	static std::mt19937 generator;
	return distribution(generator);
}

inline double clamp(double x, double min, double max) {
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

// ベクトルクラス

class vec3 {
public:
	vec3() : e{ 0,0,0 } {}
	vec3(double e0, double e1, double e2) : e{ e0, e1, e2 } {}

	double x() const { return e[0]; }
	double y() const { return e[1]; }
	double z() const { return e[2]; }

	vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
	double operator[](int i) const { return e[i]; }
	double& operator[](int i) { return e[i]; }

	vec3& operator+=(const vec3& v) {
		e[0] += v.e[0];
		e[1] += v.e[1];
		e[2] += v.e[2];
		return *this;
	}

	vec3& operator*=(const double t) {
		e[0] *= t;
		e[1] *= t;
		e[2] *= t;
		return *this;
	}

	vec3& operator/=(const double t) {
		return *this *= 1 / t;
	}

	double length() const {
		return sqrt(length_squared());
	}

	double length_squared() const {
		return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
	}

	inline static vec3 random() {
		return vec3(random_double(), random_double(), random_double());
	}

	inline static vec3 random(double min, double max) {
		return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
	}

public:
	double e[3];
};

// vec3 ユーティリティ関数

inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
	return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) {
	return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3& u, const vec3& v) {
	return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3& u, const vec3& v) {
	return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(double t, const vec3& v) {
	return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3& v, double t) {
	return t * v;
}

inline vec3 operator/(vec3 v, double t) {
	return (1 / t) * v;
}

inline double dot(const vec3& u, const vec3& v) {
	return u.e[0] * v.e[0]
		+ u.e[1] * v.e[1]
		+ u.e[2] * v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
	return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
		u.e[2] * v.e[0] - u.e[0] * v.e[2],
		u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(vec3 v) {
	return v / v.length();
}

vec3 random_in_unit_sphere() {
	while (true) {
		auto p = vec3::random(-1, 1);
		if (p.length_squared() >= 1) continue;
		return p;
	}
}

vec3 random_unit_vector() {
	auto a = random_double(0, 2 * pi);
	auto z = random_double(-1, 1);
	auto r = sqrt(1 - z * z);
	return vec3(r * cos(a), r * sin(a), z);
}

vec3 random_in_unit_disk() {
	while (true) {
		auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
		if (p.length_squared() >= 1) continue;
		return p;
	}
}

vec3 reflect(const vec3& v, const vec3& n) {
	return v - 2 * dot(v, n) * n;
}

vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat) {
	auto cos_theta = dot(-uv, n);
	vec3 r_out_parallel = etai_over_etat * (uv + cos_theta * n);
	vec3 r_out_perp = -sqrt(1.0 - r_out_parallel.length_squared()) * n;
	return r_out_parallel + r_out_perp;
}

// vec3 の型エイリアス
using point3 = vec3;   // 3D 点
using color = vec3;    // RGB 色

uint8_t* pixels = nullptr;//下方のDirectXで、ここに書き込まれた絵をテクスチャにして表示する
int idx = 0;
void write_color(std::ostream& out, color pixel_color, int samples_per_pixel) {
	auto r = pixel_color.x();
	auto g = pixel_color.y();
	auto b = pixel_color.z();

	// 色の合計をサンプルの数で割り、gamma = 2.0 のガンマ補正を行う
	auto scale = 1.0 / samples_per_pixel;
	r = sqrt(scale * r);
	g = sqrt(scale * g);
	b = sqrt(scale * b);

	// 各成分を [0,255] に変換して出力する
	pixels[idx++] = static_cast<uint8_t>(256 * clamp(r, 0.0, 0.999));
	pixels[idx++] = static_cast<uint8_t>(256 * clamp(g, 0.0, 0.999));
	pixels[idx++] = static_cast<uint8_t>(256 * clamp(b, 0.0, 0.999));
	pixels[idx++] = 255;
}

class ray {
public:
	ray() {}
	ray(const point3& origin, const vec3& direction)
		: orig(origin), dir(direction) {
	}

	point3 origin() const { return orig; }
	vec3 direction() const { return dir; }

	point3 at(double t) const {
		return orig + t * dir;
	}

public:
	point3 orig;
	vec3 dir;
};

double hit_sphere(const point3& center, double radius, const ray& r) {
	vec3 oc = r.origin() - center;
	auto a = r.direction().length_squared();
	auto half_b = dot(oc, r.direction());
	auto c = oc.length_squared() - radius * radius;
	auto discriminant = half_b * half_b - a * c;

	if (discriminant < 0) {
		return -1.0;
	}
	else {
		return (-half_b - sqrt(discriminant)) / a;
	}
}

class material;

struct hit_record {
	point3 p;
	vec3 normal;
	shared_ptr<material> mat_ptr;
	double t;
	bool front_face;

	inline void set_face_normal(const ray& r, const vec3& outward_normal) {
		front_face = dot(r.direction(), outward_normal) < 0;
		normal = front_face ? outward_normal : -outward_normal;
	}
};

class hittable {
public:
	virtual ~hittable() {}
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};

class sphere : public hittable {
public:
	sphere() {}
	sphere(point3 cen, double r, shared_ptr<material> m)
		: center(cen), radius(r), mat_ptr(m) {
	};

	virtual bool hit(const ray& r, double tmin, double tmax, hit_record& rec) const;

public:
	point3 center;
	double radius;
	shared_ptr<material> mat_ptr;
};

bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	vec3 oc = r.origin() - center;
	auto a = r.direction().length_squared();
	auto half_b = dot(oc, r.direction());
	auto c = oc.length_squared() - radius * radius;
	auto discriminant = half_b * half_b - a * c;

	if (discriminant > 0) {
		auto root = sqrt(discriminant);
		auto temp = (-half_b - root) / a;
		if (temp < t_max && temp > t_min) {
			rec.t = temp;
			rec.p = r.at(rec.t);
			vec3 outward_normal = (rec.p - center) / radius;
			rec.set_face_normal(r, outward_normal);
			rec.mat_ptr = mat_ptr;
			return true;
		}
		temp = (-half_b + root) / a;
		if (temp < t_max && temp > t_min) {
			rec.t = temp;
			rec.p = r.at(rec.t);
			vec3 outward_normal = (rec.p - center) / radius;
			rec.set_face_normal(r, outward_normal);
			rec.mat_ptr = mat_ptr;
			return true;
		}
	}
	return false;
}

class hittable_list : public hittable {
public:
	hittable_list() {}
	hittable_list(shared_ptr<hittable> object) { add(object); }

	void clear() { objects.clear(); }
	void add(shared_ptr<hittable> object) { objects.push_back(object); }

	virtual bool hit(const ray& r, double tmin, double tmax, hit_record& rec) const;

public:
	std::vector<shared_ptr<hittable>> objects;
};

bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
	hit_record temp_rec;
	bool hit_anything = false;
	auto closest_so_far = t_max;

	for (const auto& object : objects) {
		if (object->hit(r, t_min, closest_so_far, temp_rec)) {
			hit_anything = true;
			closest_so_far = temp_rec.t;
			rec = temp_rec;
		}
	}

	return hit_anything;
}

class material {
public:
	virtual ~material() {};
	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const = 0;
};

class lambertian : public material {
public:
	lambertian(const color& a) : albedo(a) {}

	virtual bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
	) const {
		vec3 scatter_direction = rec.normal + random_unit_vector();
		scattered = ray(rec.p, scatter_direction);
		attenuation = albedo;
		return true;
	}

public:
	color albedo;
};

class metal : public material {
public:
	metal(const color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

	virtual bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
	) const {
		vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);
	}

public:
	color albedo;
	double fuzz;
};

double schlick(double cosine, double ref_idx) {
	auto r0 = (1 - ref_idx) / (1 + ref_idx);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}

class dielectric : public material {
public:
	dielectric(double ri) : ref_idx(ri) {}

	virtual bool scatter(
		const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered
	) const {
		attenuation = color(1.0, 1.0, 1.0);
		double etai_over_etat;
		if (rec.front_face) {
			etai_over_etat = 1.0 / ref_idx;
		}
		else {
			etai_over_etat = ref_idx;
		}

		vec3 unit_direction = unit_vector(r_in.direction());
		double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
		double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
		if (etai_over_etat * sin_theta > 1.0) {
			vec3 reflected = reflect(unit_direction, rec.normal);
			scattered = ray(rec.p, reflected);
			return true;
		}
		double reflect_prob = schlick(cos_theta, etai_over_etat);
		if (random_double() < reflect_prob) {
			vec3 reflected = reflect(unit_direction, rec.normal);
			scattered = ray(rec.p, reflected);
			return true;
		}

		vec3 refracted = refract(unit_direction, rec.normal, etai_over_etat);
		scattered = ray(rec.p, refracted);
		return true;
	}

	double ref_idx;
};

class camera {
public:
	camera(
		point3 lookfrom,
		point3 lookat,
		vec3   vup,
		double vfov, // 垂直方向の視野角 (弧度法)
		double aspect_ratio,
		double aperture,
		double focus_dist
	) {
		auto theta = degrees_to_radians(vfov);
		auto h = tan(theta / 2);
		auto viewport_height = 2.0 * h;
		auto viewport_width = aspect_ratio * viewport_height;

		w = unit_vector(lookfrom - lookat);
		u = unit_vector(cross(vup, w));
		v = cross(w, u);

		origin = lookfrom;
		horizontal = focus_dist * viewport_width * u;
		vertical = focus_dist * viewport_height * v;
		lower_left_corner = origin - horizontal / 2 - vertical / 2 - focus_dist * w;

		lens_radius = aperture / 2;
	}

	ray get_ray(double s, double t) const {
		vec3 rd = lens_radius * random_in_unit_disk();
		vec3 offset = u * rd.x() + v * rd.y();

		return ray(
			origin + offset,
			lower_left_corner + s * horizontal + t * vertical - origin - offset
		);
	}

private:
	point3 origin;
	point3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
	vec3 u, v, w;
	double lens_radius;
};

color ray_color(const ray& r, const hittable& world, int depth) {
	hit_record rec;

	// 反射回数が一定よりも多くなったら、その時点で追跡をやめる
	if (depth <= 0)
		return color(0, 0, 0);

	if (world.hit(r, 0.001, infinity, rec)) {
		ray scattered;
		color attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation * ray_color(scattered, world, depth - 1);
		return color(0, 0, 0);
	}

	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

hittable_list random_scene() {
	hittable_list world;

	auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
	world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
				shared_ptr<material> sphere_material;

				if (choose_mat < 0.8) {
					// diffuse
					auto albedo = color::random() * color::random();
					sphere_material = make_shared<lambertian>(albedo);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95) {
					// metal
					auto albedo = color::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else {
					// glass
					sphere_material = make_shared<dielectric>(1.5);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}

	auto material1 = make_shared<dielectric>(1.5);
	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

	auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
	world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

	auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

	return world;
}

#include<Windows.h>
void debugStr(const char* format, ...)
{
	char str[256];
	va_list args;
	va_start(args, format);
	vsprintf_s(str, format, args);
	va_end(args);

	OutputDebugStringA(str);
}

const auto aspect_ratio = 16.0 / 9.0;
const int image_width = 384;
const int image_height = static_cast<int>(image_width / aspect_ratio);
void gmain() {
	const int samples_per_pixel = 100;
	const int max_depth = 50;

	hittable_list world = random_scene();

	point3 lookfrom(13, 2, 3);
	point3 lookat(0, 0, 0);
	vec3 vup(0, 1, 0);
	auto dist_to_focus = 10.0;
	auto aperture = 0.1;
	uint64_t beginTime = timeGetTime();
	camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus);

	for (int j = image_height - 1; j >= 0; --j) {
		debugStr("Scanlines remaining:%d\n", j);
		for (int i = 0; i < image_width; ++i) {
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s) {
				auto u = (i + random_double()) / (image_width - 1);
				auto v = (j + random_double()) / (image_height - 1);
				ray r = cam.get_ray(u, v);
				pixel_color += ray_color(r, world, max_depth);
			}
			write_color(std::cout, pixel_color, samples_per_pixel);
		}
	}

	debugStr("Done. %.2fsec\n",(timeGetTime()-beginTime)/1000.0f);
}


/*
uint8_t* pixels = nullptr;//下方のDirectXで、ここに書き込まれた絵をテクスチャにして表示する
int idx = 0;
void write_color(std::ostream& out, color& pixel_color) {
	pixels[idx++] = static_cast<uint8_t>(255.999 * pixel_color.x());
	pixels[idx++] = static_cast<uint8_t>(255.999 * pixel_color.y());
	pixels[idx++] = static_cast<uint8_t>(255.999 * pixel_color.z());
	pixels[idx++] = 255;
}
*/












//テクスチャ表示のための記述---------------------------------------------------------
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3d12.lib")

#include<fstream>
#include<dxgi1_6.h>
#include<d3d12.h>
#include<cmath>
#include<cassert>
#include<DirectXMath.h>
#include<wrl.h>//ComPtr

using namespace DirectX;
using namespace Microsoft::WRL;//ComPtr

void waitGPU();
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

//システム----------------------------------------------------------------------
//　ウィンドウ
LPCWSTR	WindowTitle = L"RayTracing";
int ClientWidth = image_width;
int ClientHeight = image_height;
int ClientPosX = (GetSystemMetrics(SM_CXSCREEN) - ClientWidth) / 2;//中央表示
int ClientPosY = (GetSystemMetrics(SM_CYSCREEN) - ClientHeight) / 2;//中央表示
float Aspect = static_cast<float>(ClientWidth) / ClientHeight;
#if 1 
DWORD WindowStyle = WS_OVERLAPPEDWINDOW;
#else 
DWORD WindowStyle = WS_POPUP;//Alt + F4で閉じる
#endif
HWND HWnd;
//　デバイス
ComPtr<ID3D12Device> Device;
//　コマンド
ComPtr<ID3D12CommandAllocator> CommandAllocator;
ComPtr<ID3D12GraphicsCommandList> CommandList;
ComPtr<ID3D12CommandQueue> CommandQueue;
//　フェンス
ComPtr<ID3D12Fence> Fence;
HANDLE FenceEvent;
UINT64 FenceValue;
//　デバッグ
HRESULT Hr;
//　バックバッファ
ComPtr<IDXGISwapChain4> SwapChain;
ComPtr<ID3D12Resource> BackBuffers[2];
UINT BackBufIdx;
ComPtr<ID3D12DescriptorHeap> BbvHeap;//"Bbv"は"BackBufferView"の略
UINT BbvIncSize;
float ClearColor[] = { 0.f, 0.f, 0.f, 1.0f };
//　デプスステンシルバッファ
ComPtr<ID3D12Resource> DepthStencilBuffer;
ComPtr<ID3D12DescriptorHeap> DsvHeap;//"Dsv"は"DepthStencilBufferView"の略
//　パイプライン
ComPtr<ID3D12RootSignature> RootSignature;
ComPtr<ID3D12PipelineState> PipelineState;
D3D12_VIEWPORT Viewport;
D3D12_RECT ScissorRect;

//ディスクリプタヒープ------------------------------------------------------------
ComPtr<ID3D12DescriptorHeap> CbvTbvHeap;//"Cbv"はConstBufferView、"Tbv"はTextureBufferViewの略
UINT NumCbvTbv = 1;//Viewの数によってここを変えねばならない！！！！
UINT CurrentCbvTbvIdx = 0;//Viewを１つ作ったらカウントアップしていく
UINT CbvTbvIncSize;

//メッシュリソース---------------------------------------------------------------
//　頂点バッファ
ComPtr<ID3D12Resource> VertexBuffer = nullptr;
D3D12_VERTEX_BUFFER_VIEW Vbv;
//　テクスチャバッファ
ComPtr<ID3D12Resource> TextureBuffer = nullptr;

//エントリーポイント
INT WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ INT)
{
	//システム
	{
		//ウィンドウ
		{
			//ウィンドウクラス登録
			WNDCLASSEX windowClass = {};
			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.style = CS_HREDRAW | CS_VREDRAW;
			windowClass.lpfnWndProc = WndProc;
			windowClass.hInstance = GetModuleHandle(0);
			windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
			windowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			windowClass.lpszClassName = L"GAME_WINDOW";
			RegisterClassEx(&windowClass);
			//表示位置、ウィンドウの大きさ調整
			RECT windowRect = { 0, 0, ClientWidth, ClientHeight };
			AdjustWindowRect(&windowRect, WindowStyle, FALSE);
			int windowPosX = ClientPosX + windowRect.left;
			int windowPosY = ClientPosY + windowRect.top;
			int windowWidth = windowRect.right - windowRect.left;
			int windowHeight = windowRect.bottom - windowRect.top;
			//ウィンドウをつくる
			HWnd = CreateWindowEx(
				NULL,
				L"GAME_WINDOW",
				WindowTitle,
				WindowStyle,
				windowPosX,
				windowPosY,
				windowWidth,
				windowHeight,
				NULL,		//親ウィンドウなし
				NULL,		//メニューなし
				GetModuleHandle(0),
				NULL);		//複数ウィンドウなし
		}
		//デバイス
		{
#ifdef _DEBUG
			//デバッグモードでは、デバッグレイヤーを有効化する
			ComPtr<ID3D12Debug> debug;
			Hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
			assert(SUCCEEDED(Hr));
			debug->EnableDebugLayer();
#endif
			//デバイスをつくる(簡易バージョン)
			{
				Hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0,
					IID_PPV_ARGS(Device.GetAddressOf()));
				assert(SUCCEEDED(Hr));
			}
			//コマンド
			{
				//コマンドアロケータをつくる
				Hr = Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
					IID_PPV_ARGS(CommandAllocator.GetAddressOf()));
				assert(SUCCEEDED(Hr));

				//コマンドリストをつくる
				Hr = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
					CommandAllocator.Get(), nullptr, IID_PPV_ARGS(CommandList.GetAddressOf()));
				assert(SUCCEEDED(Hr));

				//コマンドキューをつくる
				D3D12_COMMAND_QUEUE_DESC desc = {};
				desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;		//GPUタイムアウトが有効
				desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;		//直接コマンドキュー
				Hr = Device->CreateCommandQueue(&desc, IID_PPV_ARGS(CommandQueue.GetAddressOf()));
				assert(SUCCEEDED(Hr));
			}
			//フェンス
			{
				//GPUの処理完了をチェックするフェンスをつくる
				Hr = Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(Fence.GetAddressOf()));
				assert(SUCCEEDED(Hr));
				FenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
				assert(FenceEvent != nullptr);
				FenceValue = 1;
			}
		}
		//レンダーターゲット		
		{
			//スワップチェインをつくる(ここにバックバッファが含まれている)
			{
				//DXGIファクトリをつくる
				ComPtr<IDXGIFactory4> dxgiFactory;
				Hr = CreateDXGIFactory2(0, IID_PPV_ARGS(dxgiFactory.GetAddressOf()));
				assert(SUCCEEDED(Hr));

				//スワップチェインをつくる
				DXGI_SWAP_CHAIN_DESC1 desc = {};
				desc.BufferCount = 2; //バックバッファ2枚
				desc.Width = ClientWidth;
				desc.Height = ClientHeight;
				desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				desc.SampleDesc.Count = 1;
				ComPtr<IDXGISwapChain1> swapChain1;
				Hr = dxgiFactory->CreateSwapChainForHwnd(
					CommandQueue.Get(), HWnd, &desc, nullptr, nullptr, swapChain1.GetAddressOf());
				assert(SUCCEEDED(Hr));

				//IDXGISwapChain4インターフェイスをサポートしているか尋ねる
				Hr = swapChain1->QueryInterface(IID_PPV_ARGS(SwapChain.GetAddressOf()));
				assert(SUCCEEDED(Hr));
			}
			//バックバッファ「ビュー」の入れ物である「ディスクリプタヒープ」をつくる
			{
				D3D12_DESCRIPTOR_HEAP_DESC desc = {};
				desc.NumDescriptors = 2;//バックバッファビュー２つ
				desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//RenderTargetView
				desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;//シェーダからアクセスしないのでNONEでOK
				Hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(BbvHeap.GetAddressOf()));
				assert(SUCCEEDED(Hr));
			}
			//バックバッファ「ビュー」を「ディスクリプタヒープ」につくる
			{
				D3D12_CPU_DESCRIPTOR_HANDLE hBbvHeap
					= BbvHeap->GetCPUDescriptorHandleForHeapStart();

				BbvIncSize
					= Device->GetDescriptorHandleIncrementSize(
						D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

				for (UINT idx = 0; idx < 2; idx++) {
					//バックバッファを取り出す
					Hr = SwapChain->GetBuffer(idx, IID_PPV_ARGS(BackBuffers[idx].GetAddressOf()));
					assert(SUCCEEDED(Hr));
					//バックバッファのビューをヒープにつくる
					hBbvHeap.ptr += BbvIncSize * idx;
					Device->CreateRenderTargetView(BackBuffers[idx].Get(), nullptr, hBbvHeap);
				}
			}
			//デプスステンシルバッファをつくる
			{
				D3D12_HEAP_PROPERTIES prop = {};
				prop.Type = D3D12_HEAP_TYPE_DEFAULT;//DEFAULTだから後はUNKNOWNでよし
				prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				D3D12_RESOURCE_DESC desc = {};
				desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元のテクスチャデータとして
				desc.Width = ClientWidth;//幅と高さはレンダーターゲットと同じ
				desc.Height = ClientHeight;//上に同じ
				desc.DepthOrArraySize = 1;//テクスチャ配列でもないし3Dテクスチャでもない
				desc.Format = DXGI_FORMAT_D32_FLOAT;//深度値書き込み用フォーマット
				desc.SampleDesc.Count = 1;//サンプルは1ピクセル当たり1つ
				desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//このバッファは深度ステンシルとして使用します
				desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				desc.MipLevels = 1;
				//デプスステンシルバッファをクリアする値
				D3D12_CLEAR_VALUE depthClearValue = {};
				depthClearValue.DepthStencil.Depth = 1.0f;//深さ１(最大値)でクリア
				depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;//32bit深度値としてクリア
				//デプスステンシルバッファを作る
				Hr = Device->CreateCommittedResource(
					&prop,
					D3D12_HEAP_FLAG_NONE,
					&desc,
					D3D12_RESOURCE_STATE_DEPTH_WRITE, //デプス書き込みに使用
					&depthClearValue,
					IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf()));
				assert(SUCCEEDED(Hr));
			}
			//デプスステンシルバッファ「ビュー」の入れ物である「デスクリプタヒープ」をつくる
			{
				D3D12_DESCRIPTOR_HEAP_DESC desc = {};//深度に使うよという事がわかればいい
				desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;//デプスステンシルビューとして使う
				desc.NumDescriptors = 1;//深度ビュー1つのみ
				desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				Hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(DsvHeap.GetAddressOf()));
				assert(SUCCEEDED(Hr));
			}
			//デプスステンシルバッファ「ビュー」を「ディスクリプタヒープ」につくる
			{
				D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
				desc.Format = DXGI_FORMAT_D32_FLOAT;//デプス値に32bit使用
				desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2Dテクスチャ
				desc.Flags = D3D12_DSV_FLAG_NONE;//フラグは特になし
				D3D12_CPU_DESCRIPTOR_HANDLE hDsvHeap
					= DsvHeap->GetCPUDescriptorHandleForHeapStart();
				Device->CreateDepthStencilView(DepthStencilBuffer.Get(), &desc, hDsvHeap);
			}
		}{}
		//ルートシグネチャ
		{
			//ディスクリプタレンジ。ディスクリプタヒープとシェーダを紐づける役割をもつ。
			D3D12_DESCRIPTOR_RANGE  range[1] = {};
			UINT t0 = 0;
			range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			range[0].BaseShaderRegister = t0;
			range[0].NumDescriptors = 1;
			range[0].RegisterSpace = 0;
			range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			//ルートパラメタをディスクリプタテーブルとして使用
			D3D12_ROOT_PARAMETER rootParam[1] = {};
			rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParam[0].DescriptorTable.pDescriptorRanges = range;
			rootParam[0].DescriptorTable.NumDescriptorRanges = _countof(range);
			rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

			//サンプラの記述。このサンプラがシェーダーの s0 にセットされる
			D3D12_STATIC_SAMPLER_DESC samplerDesc[1] = {};
			samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//補間しない(ニアレストネイバー)
			samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//横繰り返し
			samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//縦繰り返し
			samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//奥行繰り返し
			samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//ボーダーの時は黒
			samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;//ミップマップ最大値
			samplerDesc[0].MinLOD = 0.0f;//ミップマップ最小値
			samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//オーバーサンプリングの際リサンプリングしない？
			samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//ピクセルシェーダからのみ可視

			//ルートシグニチャの記述
			D3D12_ROOT_SIGNATURE_DESC desc = {};
			desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
			desc.pParameters = rootParam;
			desc.NumParameters = _countof(rootParam);
			desc.pStaticSamplers = samplerDesc;//サンプラーの先頭アドレス
			desc.NumStaticSamplers = _countof(samplerDesc);//サンプラー数

			//ルートシグネチャをシリアライズ⇒blob(塊)をつくる。
			ComPtr<ID3DBlob> blob;
			Hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr);
			assert(SUCCEEDED(Hr));

			//ルートシグネチャをつくる
			Hr = Device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
				IID_PPV_ARGS(RootSignature.GetAddressOf()));
			assert(SUCCEEDED(Hr));
		}
		//パイプライン
		{
			//コンパイル済みシェーダを読み込むファイルバッファ
			class BIN_FILE12 {
			public:
				BIN_FILE12(const char* fileName) :Succeeded(false)
				{
					std::ifstream ifs(fileName, std::ios::binary);
					if (ifs.fail()) {
						return;
					}
					Succeeded = true;
					std::istreambuf_iterator<char> first(ifs);
					std::istreambuf_iterator<char> last;
					Buffer.assign(first, last);
					ifs.close();
				}
				bool succeeded() const
				{
					return Succeeded;
				}
				unsigned char* code() const
				{
					char* p = const_cast<char*>(Buffer.data());
					return reinterpret_cast<unsigned char*>(p);
				}
				size_t size() const
				{
					return Buffer.size();
				}
			private:
				std::string Buffer;
				bool Succeeded;
			};
			//シェーダ読み込み
			BIN_FILE12 vs("assets\\VertexShader.cso");
			assert(vs.succeeded());
			BIN_FILE12 ps("assets\\PixelShader.cso");
			assert(ps.succeeded());

			//以下、各種記述

			UINT slot0 = 0;
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, slot0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    slot0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};

			D3D12_RASTERIZER_DESC rasterDesc = {};
			rasterDesc.FrontCounterClockwise = true;
			rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
			rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
			rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
			rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
			rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			rasterDesc.DepthClipEnable = true;
			rasterDesc.MultisampleEnable = false;
			rasterDesc.AntialiasedLineEnable = false;
			rasterDesc.ForcedSampleCount = 0;
			rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

			D3D12_BLEND_DESC blendDesc = {};
			blendDesc.AlphaToCoverageEnable = true;
			blendDesc.IndependentBlendEnable = false;
			blendDesc.RenderTarget[0].BlendEnable = true;
			blendDesc.RenderTarget[0].LogicOpEnable = false;
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

			D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
			depthStencilDesc.DepthEnable = true;
			depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;//全て書き込み
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;//小さい方を採用
			depthStencilDesc.StencilEnable = false;//ステンシルバッファは使わない


			//ここまでの記述をまとめてパイプラインステートオブジェクトをつくる
			D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineDesc = {};
			pipelineDesc.pRootSignature = RootSignature.Get();
			pipelineDesc.VS = { vs.code(), vs.size() };
			pipelineDesc.PS = { ps.code(), ps.size() };
			pipelineDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
			pipelineDesc.RasterizerState = rasterDesc;
			pipelineDesc.BlendState = blendDesc;
			pipelineDesc.DepthStencilState = depthStencilDesc;
			pipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			pipelineDesc.SampleMask = UINT_MAX;
			pipelineDesc.SampleDesc.Count = 1;
			pipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			pipelineDesc.NumRenderTargets = 1;
			pipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			Hr = Device->CreateGraphicsPipelineState(
				&pipelineDesc,
				IID_PPV_ARGS(PipelineState.GetAddressOf())
			);
			assert(SUCCEEDED(Hr));

			//出力領域を設定
			Viewport.TopLeftX = 0.0f;
			Viewport.TopLeftY = 0.0f;
			Viewport.Width = (float)ClientWidth;
			Viewport.Height = (float)ClientHeight;
			Viewport.MinDepth = 0.0f;
			Viewport.MaxDepth = 1.0f;

			//切り取り矩形を設定
			ScissorRect.left = 0;
			ScissorRect.top = 0;
			ScissorRect.right = ClientWidth;
			ScissorRect.bottom = ClientHeight;
		}
		//ウィンドウ表示
		ShowWindow(HWnd, SW_SHOW);
	}{}
	//ディスクリプタヒープ（コンスタントバッファビュー＆テクスチャバッファビューの入れ物）
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = NumCbvTbv;
		desc.NodeMask = 0;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		Hr = Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(CbvTbvHeap.ReleaseAndGetAddressOf()));
		assert(SUCCEEDED(Hr));

		//ハンドル(ポインタ)増分サイズを取得しておく
		CbvTbvIncSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	//メッシュリソース
	{
		//頂点バッファ
		{
			float Vertices[] = {
				-1.0f,  1.0f,  0.0f,  0.0f,  0.0f, //左上
				-1.0f, -1.0f,  0.0f,  0.0f,  1.0f, //左下
				 1.0f,  1.0f,  0.0f,  1.0f,  0.0f, //右上
				 1.0f, -1.0f,  0.0f,  1.0f,  1.0f, //右下
			};
			int NumVertexElements = 5;
			//サイズ計算
			UINT sizeInBytes = sizeof(Vertices);
			UINT strideInBytes = sizeof(float) * NumVertexElements;
			//位置のバッファをつくる
			{
				D3D12_HEAP_PROPERTIES prop = {};
				prop.Type = D3D12_HEAP_TYPE_UPLOAD;
				prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				prop.CreationNodeMask = 1;
				prop.VisibleNodeMask = 1;
				D3D12_RESOURCE_DESC desc = {};
				desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				desc.Alignment = 0;
				desc.Width = sizeInBytes;
				desc.Height = 1;
				desc.DepthOrArraySize = 1;
				desc.MipLevels = 1;
				desc.Format = DXGI_FORMAT_UNKNOWN;
				desc.SampleDesc = { 1, 0 };
				desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				desc.Flags = D3D12_RESOURCE_FLAG_NONE;
				Hr = Device->CreateCommittedResource(
					&prop,
					D3D12_HEAP_FLAG_NONE,
					&desc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(VertexBuffer.ReleaseAndGetAddressOf()));
				assert(SUCCEEDED(Hr));
			}
			//位置バッファに生データをコピー
			{
				UINT8* mappedBuf;
				Hr = VertexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedBuf));
				assert(SUCCEEDED(Hr));
				memcpy(mappedBuf, Vertices, sizeInBytes);
				VertexBuffer->Unmap(0, nullptr);
			}
			//位置バッファのビューを初期化しておく。（ディスクリプタヒープに作らなくてよい）
			{
				Vbv.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
				Vbv.SizeInBytes = sizeInBytes;
				Vbv.StrideInBytes = strideInBytes;
			}
		}
		//テクスチャバッファ
		{
			//バッファをつくって、pixelsデータを入れる
			{
				//pixelsを作る
				int bytePerPixel = 4;
				pixels = new uint8_t[image_width * image_height * bytePerPixel];
				gmain();

				//１行のピッチを256の倍数にしておく(バッファサイズは256の倍数でなければいけない)
				const UINT64 alignedRowPitch = (image_width * bytePerPixel + 0xff) & ~0xff;

				//アップロード用中間バッファをつくり、生データをコピーしておく
				ComPtr<ID3D12Resource> uploadBuf;
				{
					//テクスチャではなくフツーのバッファとしてつくる
					D3D12_HEAP_PROPERTIES prop = {};
					prop.Type = D3D12_HEAP_TYPE_UPLOAD;
					prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
					prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
					prop.CreationNodeMask = 1;
					prop.VisibleNodeMask = 1;
					D3D12_RESOURCE_DESC desc = {};
					desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
					desc.Alignment = 0;
					desc.Width = alignedRowPitch * image_height;
					desc.Height = 1;
					desc.DepthOrArraySize = 1;
					desc.MipLevels = 1;
					desc.Format = DXGI_FORMAT_UNKNOWN;
					desc.SampleDesc = { 1, 0 };
					desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
					desc.Flags = D3D12_RESOURCE_FLAG_NONE;
					Hr = Device->CreateCommittedResource(
						&prop,
						D3D12_HEAP_FLAG_NONE,
						&desc,
						D3D12_RESOURCE_STATE_GENERIC_READ,
						nullptr,
						IID_PPV_ARGS(&uploadBuf));
					assert(SUCCEEDED(Hr));

					//生データをuploadbuffに一旦コピーします
					uint8_t* mapBuf = nullptr;
					Hr = uploadBuf->Map(0, nullptr, (void**)&mapBuf);//マップ
					auto srcAddress = pixels;
					auto originalRowPitch = image_width * bytePerPixel;
					for (int y = 0; y < image_height; ++y) {
						memcpy(mapBuf, srcAddress, originalRowPitch);
						//1行ごとの辻褄を合わせてやる
						mapBuf += alignedRowPitch;
						srcAddress += originalRowPitch;
					}
					uploadBuf->Unmap(0, nullptr);//アンマップ
				}

				//そして、最終コピー先であるテクスチャバッファを作る
				{
					D3D12_HEAP_PROPERTIES prop = {};
					prop.Type = D3D12_HEAP_TYPE_DEFAULT;//CPUからアクセスしない。処理が速い。
					prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
					prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
					prop.CreationNodeMask = 1;
					prop.VisibleNodeMask = 1;
					D3D12_RESOURCE_DESC desc = {};
					desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//他のバッファと違う
					desc.Alignment = 0;
					desc.Width = image_width;
					desc.Height = image_height;//他のバッファと違う
					desc.DepthOrArraySize = 1;
					desc.MipLevels = 1;
					desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//他のバッファと違う
					desc.SampleDesc = { 1, 0 };
					desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//他のバッファと違う
					desc.Flags = D3D12_RESOURCE_FLAG_NONE;
					Hr = Device->CreateCommittedResource(
						&prop,
						D3D12_HEAP_FLAG_NONE,
						&desc,
						D3D12_RESOURCE_STATE_COPY_DEST,
						nullptr,
						IID_PPV_ARGS(TextureBuffer.ReleaseAndGetAddressOf()));
					assert(SUCCEEDED(Hr));
				}

				//GPUでuploadBufからtextureBufへコピーする長い道のりが始まります

				//まずコピー元ロケーションの準備・フットプリント指定
				D3D12_TEXTURE_COPY_LOCATION src = {};
				src.pResource = uploadBuf.Get();
				src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				src.PlacedFootprint.Footprint.Width = static_cast<UINT>(image_width);
				src.PlacedFootprint.Footprint.Height = static_cast<UINT>(image_height);
				src.PlacedFootprint.Footprint.Depth = static_cast<UINT>(1);
				src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(alignedRowPitch);
				src.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				//コピー先ロケーションの準備・サブリソースインデックス指定
				D3D12_TEXTURE_COPY_LOCATION dst = {};
				dst.pResource = TextureBuffer.Get();
				dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				dst.SubresourceIndex = 0;

				//コマンドリストでコピーを予約しますよ！！！
				CommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
				//ってことはバリアがいるのです
				D3D12_RESOURCE_BARRIER barrier = {};
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.pResource = TextureBuffer.Get();
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
				barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
				CommandList->ResourceBarrier(1, &barrier);
				//uploadBufアンロード
				CommandList->DiscardResource(uploadBuf.Get(), nullptr);
				//コマンドリストを閉じて
				CommandList->Close();
				//実行
				ID3D12CommandList* commandLists[] = { CommandList.Get() };
				CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
				//リソースがGPUに転送されるまで待機する
				waitGPU();

				//コマンドアロケータをリセット
				HRESULT Hr = CommandAllocator->Reset();
				assert(SUCCEEDED(Hr));
				//コマンドリストをリセット
				Hr = CommandList->Reset(CommandAllocator.Get(), nullptr);
				assert(SUCCEEDED(Hr));

				//開放
				delete[]pixels;
			}{}
			//テクスチャバッファの「ビュー」を「ディスクリプタヒープ」につくる
			{
				auto hCbvTbvHeap = CbvTbvHeap->GetCPUDescriptorHandleForHeapStart();
				hCbvTbvHeap.ptr += CbvTbvIncSize * CurrentCbvTbvIdx++;//計算後カウントアップ

				D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
				desc.Format = TextureBuffer->GetDesc().Format;
				desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
				desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
				desc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1
				Device->CreateShaderResourceView(TextureBuffer.Get(), &desc, hCbvTbvHeap);
			}
		}
	}{}
	//描画前処理
	{
		//現在のバックバッファのインデックスを取得。このプログラムの場合0 or 1になる。
		BackBufIdx = SwapChain->GetCurrentBackBufferIndex();
		//バリアでバックバッファを描画ターゲットに切り替える
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;//このバリアは状態遷移タイプ
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = BackBuffers[BackBufIdx].Get();//リソースはバックバッファ
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;//遷移前はPresent
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;//遷移後は描画ターゲット
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		CommandList->ResourceBarrier(1, &barrier);
		//バックバッファの場所を指すディスクリプタヒープハンドルを用意する
		auto hBbvHeap = BbvHeap->GetCPUDescriptorHandleForHeapStart();
		hBbvHeap.ptr += BackBufIdx * BbvIncSize;
		//デプスステンシルバッファのディスクリプタハンドルを用意する
		auto hDsvHeap = DsvHeap->GetCPUDescriptorHandleForHeapStart();
		//バックバッファとデプスステンシルバッファを描画ターゲットとして設定する
		CommandList->OMSetRenderTargets(1, &hBbvHeap, false, &hDsvHeap);
		//描画ターゲットをクリアする
		CommandList->ClearRenderTargetView(hBbvHeap, ClearColor, 0, nullptr);
		//デプスステンシルバッファをクリアする
		CommandList->ClearDepthStencilView(hDsvHeap, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		//ビューポートとシザー矩形をセット
		CommandList->RSSetViewports(1, &Viewport);
		CommandList->RSSetScissorRects(1, &ScissorRect);
		//パイプラインステートをセット
		CommandList->SetPipelineState(PipelineState.Get());
		//ルートシグニチャをセット
		CommandList->SetGraphicsRootSignature(RootSignature.Get());
		//ディスクリプタヒープをＧＰＵにセット
		//                                 ↓ここだけは "&CbvTbvHeap" ではだめだった
		CommandList->SetDescriptorHeaps(1, CbvTbvHeap.GetAddressOf());
	}
	//メッシュを描画
	{
		//頂点をセット
		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		CommandList->IASetVertexBuffers(0, 1, &Vbv);
		//ディスクリプタヒープをディスクリプタテーブルにセット
		auto hCbvTbvHeap = CbvTbvHeap->GetGPUDescriptorHandleForHeapStart();
		CommandList->SetGraphicsRootDescriptorTable(0, hCbvTbvHeap);
		//描画
		CommandList->DrawInstanced(4, 1, 0, 0);
	}
	//描画終了処理
	{
		//バリアでバックバッファを表示用に切り替える
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;//このバリアは状態遷移タイプ
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = BackBuffers[BackBufIdx].Get();//リソースはバックバッファ
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;//遷移前は描画ターゲット
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;//遷移後はPresent
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		CommandList->ResourceBarrier(1, &barrier);
		//コマンドリストをクローズする
		CommandList->Close();
		//コマンドリストを実行する
		ID3D12CommandList* commandLists[] = { CommandList.Get() };
		CommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);
		//描画完了を待つ
		waitGPU();
		//バックバッファを表示
		SwapChain->Present(1, 0);
		//コマンドアロケータをリセット
		Hr = CommandAllocator->Reset();
		assert(SUCCEEDED(Hr));
		//コマンドリストをリセット
		Hr = CommandList->Reset(CommandAllocator.Get(), nullptr);
		assert(SUCCEEDED(Hr));
	}
	//メインループ
	MSG msg = { 0 };
	while (true)
	{
		//ウィンドウメッセージの取得、送出
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}
		Sleep(1);
	}
	//解放
	{
		waitGPU();
		CloseHandle(FenceEvent);
	}
	return (int)msg.wParam;
}

//描画完了を待つ
void waitGPU()
{
	//現在のFence値がコマンド終了後にFenceに書き込まれるようにする
	UINT64 fvalue = FenceValue;
	CommandQueue->Signal(Fence.Get(), fvalue);
	FenceValue++;

	//まだコマンドキューが終了していないことを確認する
	if (Fence->GetCompletedValue() < fvalue)
	{
		//このFenceにおいて、fvalue の値になったらイベントを発生させる
		Fence->SetEventOnCompletion(fvalue, FenceEvent);
		//イベントが発生するまで待つ
		WaitForSingleObject(FenceEvent, INFINITE);
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}
}