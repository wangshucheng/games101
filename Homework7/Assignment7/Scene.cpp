//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here

    // 可能用到的函数有：
    // • intersect(const Ray ray)in Scene.cpp: 求一条光线与场景的交点
    // • sampleLight(Intersection pos, float pdf) in Scene.cpp: 在场景的所有光源上按面积 uniform 地 sample 一个点，并计算该 sample 的概率密度
    // • sample(const Vector3f wi, const Vector3f N) in Material.cpp: 按照该材质的性质，给定入射方向与法向量，用某种分布采样一个出射方向
    // • pdf(const Vector3f wi, const Vector3f wo, const Vector3f N) in Material.cpp: 给定一对入射、出射方向与法向量，计算 sample 方法得到该出射方向的概率密度
    // • eval(const Vector3f wi, const Vector3f wo, const Vector3f N) in Material.cpp: 给定一对入射、出射方向与法向量，计算这种情况下的 f_r 值
    // 可能用到的变量有：
    // • RussianRoulette in Scene.cpp: P_RR, Russian Roulette 的概率

    Vector3f L_dir{0,0,0}, L_indir = {0, 0, 0};

    auto obj_intersection = intersect(ray);
    if (!obj_intersection.happened)
        return 0;

    auto material = obj_intersection.m;
    // hit the light
    if (material->hasEmission())
    {
        // if (depth == 0)
        // {
            return material->getEmission();
        // }
        // else
        //     return 0;
    }

    Intersection light_intersection;
    float pdf_light = 0.0f;
    // 随机光源采样(代替对光源进行积分的计算)，其结果判断是否击中光源
    sampleLight(light_intersection, pdf_light);
    
    Vector3f obj2light = light_intersection.coords - obj_intersection.coords;
    Vector3f ws = obj2light.normalized();
    Vector3f& N = obj_intersection.normal;
    Vector3f wo = ray.direction;
    Vector3f& NN = light_intersection.normal;
    float distancePow2 = obj2light.x * obj2light.x + obj2light.y * obj2light.y + obj2light.z * obj2light.z;

    Ray obj2lightray = {obj_intersection.coords, ws};
    Intersection t = intersect(obj2lightray);
    // 判断光线是否被挡的边界情况。
    if(t.distance - obj2light.norm() > -EPSILON)
    // if(t.happened && (t.coords - light_intersection.coords).norm() < EPSILON)
    {
        Vector3f f_r = material->eval(wo, ws, N);
        // L_dir = emit * eval(wo, ws, N) * dot(ws, N) * dot(ws, NN) / |x-p|^2 / pdf_light
        L_dir = light_intersection.emit * f_r * dotProduct(ws, N) * dotProduct(-ws, NN) / distancePow2 / pdf_light;
    }

    // RR
    if(get_random_float() > RussianRoulette)
    {
        return L_dir;
    }

    Vector3f wi = material->sample(wo, N).normalized();
    Ray obj2nextobjray = {obj_intersection.coords, wi};
    Intersection t2 = intersect(obj2nextobjray);
    if(t2.happened && !t2.m->hasEmission())
    {
        float pdf = material->pdf(wo, wi, N);
        Vector3f f_r = material->eval(wo, wi, N);
        // L_indir = shade(q, wi) * eval(wo, wi, N) * dot(wi, N) / pdf(wo, wi, N) / RussianRoulette
        L_indir = castRay(obj2nextobjray, depth + 1) * f_r * dotProduct(wi, N) / pdf / RussianRoulette;
    }

    return L_dir + L_indir;
}