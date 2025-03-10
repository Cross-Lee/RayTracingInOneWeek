#include "Header/color.h"
#include "Header/vec3.h"
#include "Header/ray.h"
#include "Header/rtweekend.h"
#include "Header/hittable_list.h"
#include "Header/sphere.h"
#include "Header/camera.h"
#include "Header/material.h"
#include "Header/moving_sphere.h"
#include "Header/bvh.h"
#include "Header/aarect.h"
#include "Header/box.h"
#include "Header/constant_medium.h"
#include "Header/bvh.h"
#include <iostream>
#include <fstream>
#include <sstream>

// #define STB_IMAGE_IMPLEMENTATION
// #include "Header/stb_image.h"

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Header/stb_image_write.h"

#define CHANNEL_NUM 3
#define RR 0.5

color ray_color(
    const ray& r, 
    const color& background, 
    const hittable& world,
    shared_ptr<hittable>& lights, 
    int depth
) {
    vec3 L_dir(0, 0, 0), L_indir(0, 0, 0);
    hit_record rec;

    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0, 0, 0);

    // If the ray hits nothing, return the background color.
    if (!world.hit(r, 0.001, infinity, rec))
        return background;

    scatter_record srec;
    color emitted = rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
    
    if (!rec.mat_ptr->scatter(r, rec, srec))
    {
        L_dir = emitted;
        return L_dir;
    }

    if (srec.is_specular) {
        L_dir = srec.attenuation
             * ray_color(srec.specular_ray, background, world, lights, depth-1);
        return L_dir;
    }

    // 101中的RR是根据p点无差异进行着色，本文中的着色是根据判断着色的，所以不适合用RR
    // if(random_double() > RR)
    // {
    //     return L_dir;
    // }

    auto light_ptr = make_shared<hittable_pdf>(lights, rec.p);
    mixture_pdf p(light_ptr, srec.pdf_ptr);

    ray scattered = ray(rec.p, p.generate(), r.time());
    auto pdf_val = p.value(scattered.direction());

    return emitted + srec.attenuation * rec.mat_ptr->scattering_pdf(r, rec, scattered)
                           * ray_color(scattered, background, world, lights, depth-1) / pdf_val / RR;

    // return L_dir + L_indir;
}

hittable_list cornell_box()
{
    hittable_list objects;

    auto red = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(15, 15, 15));

    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
    objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
    objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
    objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
    objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

    shared_ptr<material> aluminum = make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
    shared_ptr<hittable> box1 = make_shared<box>(point3(0,0,0), point3(165,330,165), aluminum);
    box1 = make_shared<rotate_y>(box1, 15);
    box1 = make_shared<translate>(box1, vec3(265, 0, 295));
    objects.add(box1);

    auto glass = make_shared<dielectric>(1.5);
    objects.add(make_shared<sphere>(point3(190,90,190), 90 , glass));

    return objects;
}

int main()
{
    // Image (aspect_ratio, image_width, image_height, samples_per_pixel, max_depth)
    const auto aspect_ratio = 1.0 / 1.0;
    const int image_width = 600;
    const int image_height = static_cast<int>(image_width / aspect_ratio);
    const int samples_per_pixel = 5;
    const int max_depth = 50;

    // World (cornell_box(), background)
    auto world = cornell_box();
    bvh_node bvh_root(world, 0, 1);
    // 添加多个灯光没有解决
    // auto lights = make_shared<hittable_list>();
    // lights->add(make_shared<xz_rect>(213, 343, 227, 332, 554, shared_ptr<material>()));
    // lights->add(make_shared<sphere>(point3(190, 90, 190), 90, shared_ptr<material>()));
    // shared_ptr<hittable> lights = make_shared<xz_rect>(213, 343, 227, 332, 554, shared_ptr<material>());
    shared_ptr<hittable> lights = make_shared<sphere>(point3(190, 90, 190), 90, shared_ptr<material>());
    color background(0, 0, 0);

    // Camera (lookfrom, lookat, vup, dist_to_focus, aperture, vfov, time0, time1)
    point3 lookfrom(278, 278, -800);
    point3 lookat(278, 278, 0);
    vec3 vup(0, 1, 0);
    auto dist_to_focus = 10.0;
    auto aperture = 0.0;
    auto vfov = 40.0;
    auto time0 = 0.0;
    auto time1 = 1.0;
    camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, time0, time1);

    // // png prepare
    // uint8_t* pixels = new uint8_t[image_height * image_width * CHANNEL_NUM];
    // int index = 0;
    // vec3 finalColor;

    // Render (image_width, image_height, samples_per_pixel)
    cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
    for (int j = image_height - 1; j >= 0; --j)
    {
        cerr << "\rScanlines remaining: " << j << ' ' << flush;
        for (int i = 0; i < image_width; ++i)
        {
            color pixel_color(0, 0, 0);
            for (int s = 0; s < samples_per_pixel; ++s)
            {
                auto u = (i + random_double()) / (image_width - 1);
                auto v = (j + random_double()) / (image_height - 1);
                ray r = cam.get_ray(u, v);
                pixel_color += ray_color(r, background, bvh_root, lights, max_depth);
            }
            write_color(std::cout, pixel_color, samples_per_pixel);
            // pixels[index++] = write_png_color(pixel_color, samples_per_pixel)[0];
            // cout << pixels[index] << endl;
            // pixels[index++] = write_png_color(pixel_color, samples_per_pixel)[1];
            // cout << pixels[index] << endl;
            // pixels[index++] = write_png_color(pixel_color, samples_per_pixel)[2];
            // cout << pixels[index] << endl;
        }
    }
    // stbi_write_png("new.png", image_width, image_height, CHANNEL_NUM, pixels, image_width * CHANNEL_NUM);
    // delete[] pixels;

    cerr << "\nDone.\n";
}