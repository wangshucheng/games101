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

    if (material->hasEmission())
        return material->getEmission();

    Intersection light_intersection;
    float pdf_light = 0.0f;
    sampleLight(light_intersection, pdf_light);
    
    Vector3f obj2light = light_intersection.coords - obj_intersection.coords;
    Vector3f ws = obj2light.normalized();
    Vector3f N = obj_intersection.normal;
    Vector3f wo = ray.direction;
    Vector3f NN = light_intersection.normal;
    float distancePow2 = obj2light.x * obj2light.x + obj2light.y * obj2light.y + obj2light.z * obj2light.z;

    Ray obj2lightray = {obj_intersection.coords, ws};
    Intersection t = intersect(obj2lightray);
    if(t.distance - obj2light.norm() < EPSILON)
    {
        L_dir = light_intersection.emit * material->eval(wo, ws, N) * dotProduct(ws, N) * dotProduct(-ws, NN) / distancePow2 / pdf_light;
    }

    if(get_random_float() > RussianRoulette)
    {
        return L_dir;
    }

    Vector3f wi = material->sample(wo, N).normalized();
    Ray obj2nextobjray = {obj_intersection.coords, wi};
    Intersection nextObj_intersection = intersect(obj2nextobjray);
    if(nextObj_intersection.happened && !nextObj_intersection.m->hasEmission())
    {
        float pdf = material->pdf(wo, wi, N);
        Vector3f f_r = material->eval(wo, wi, N);
        L_indir = castRay(obj2nextobjray, depth + 1) * f_r * dotProduct(wi, N) / pdf / RussianRoulette;
    }

    return L_dir + L_indir;
}

// Vector3f Scene::castRay(const Ray &ray, int depth) const
// {
//      Vector3f L_dir = { 0,0,0 };
//      Vector3f L_indir = { 0,0,0 };
 
//      Intersection intersection = Scene::intersect(ray);
//      if (!intersection.happened)
//      {
//          return {};
//      }
//      //打到光源
//      if (intersection.m->hasEmission())
//      return intersection.m->getEmission();
 
//      //打到物体后对光源均匀采样
//      Intersection lightpos;
//      float lightpdf = 0.0f;
//      sampleLight(lightpos, lightpdf);//获得对光源的采样，包括光源的位置和采样的pdf
 
//      Vector3f collisionlight = lightpos.coords - intersection.coords;
//      float dis = collisionlight.x*collisionlight.x + collisionlight.y*collisionlight.y + collisionlight.z*collisionlight.z;
//      Vector3f collisionlightdir = collisionlight.normalized();
//      Ray objray(intersection.coords, collisionlightdir);
 
//      Intersection ishaveobj = Scene::intersect(objray);
//      //L_dir = L_i * f_r * cos_theta * cos_theta_x / |x - p | ^ 2 / pdf_light
//      if (ishaveobj.distance - collisionlight.norm() > -EPSILON)//说明之间没有遮挡
//      L_dir = lightpos.emit * intersection.m->eval(ray.direction, collisionlightdir, intersection.normal) * dotProduct(collisionlightdir, intersection.normal) * dotProduct(-collisionlightdir, lightpos.normal) / dis / lightpdf;
//  	 //打到物体后对半圆随机采样使用RR算法
//      if (get_random_float() > RussianRoulette)
//      return L_dir;
 
//      Vector3f w0 = intersection.m->sample(ray.direction, intersection.normal).normalized();
//      Ray objrayobj(intersection.coords, w0);
//      Intersection islight = Scene::intersect(objrayobj);
//      // shade(q, wi) * f_r * cos_theta / pdf_hemi / P_RR
//      if (islight.happened && !islight.m->hasEmission())
//      {
//          float pdf = intersection.m->pdf(ray.direction, w0, intersection.normal);
//          L_indir = castRay(objrayobj, depth + 1) * intersection.m->eval(ray.direction, w0, intersection.normal) * dotProduct(w0, intersection.normal) / pdf / RussianRoulette;
//      }
 
//      return L_dir + L_indir;
// }

// Vector3f Scene::castRay(const Ray &ray, int depth) const
// {
// 	Intersection inter = intersect(ray);

// 	if (inter.happened)
// 	{
// 		// 如果射线第一次打到光源，直接返回
// 		if (inter.m->hasEmission())
// 		{
// 			if (depth == 0) 
// 			{
// 				return inter.m->getEmission();
// 			}
// 			else return Vector3f(0, 0, 0);
// 		}

// 		Vector3f L_dir(0, 0, 0);
// 		Vector3f L_indir(0, 0, 0);

// 		// 随机 sample 灯光，用该 sample 的结果判断射线是否击中光源
// 		Intersection lightInter;
// 		float pdf_light = 0.0f;
// 		sampleLight(lightInter, pdf_light);

// 		// 物体表面法线
// 		auto& N = inter.normal;
// 		// 灯光表面法线
// 		auto& NN = lightInter.normal;

// 		auto& objPos = inter.coords;
// 		auto& lightPos = lightInter.coords;

// 		auto diff = lightPos - objPos;
// 		auto lightDir = diff.normalized();
// 		float lightDistance = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

// 		Ray light(objPos, lightDir);
// 		Intersection light2obj = intersect(light);

// 		// 如果反射击中光源
// 		if (light2obj.happened && (light2obj.coords - lightPos).norm() < 1e-2)
// 		{
// 			Vector3f f_r = inter.m->eval(ray.direction, lightDir, N);
// 			L_dir = lightInter.emit * f_r * dotProduct(lightDir, N) * dotProduct(-lightDir, NN) / lightDistance / pdf_light;
// 		}

// 		if (get_random_float() < RussianRoulette)
// 		{
// 			Vector3f nextDir = inter.m->sample(ray.direction, N).normalized();

// 			Ray nextRay(objPos, nextDir);
// 			Intersection nextInter = intersect(nextRay);
// 			if (nextInter.happened && !nextInter.m->hasEmission())
// 			{
// 				float pdf = inter.m->pdf(ray.direction, nextDir, N);
// 				Vector3f f_r = inter.m->eval(ray.direction, nextDir, N);
// 				L_indir = castRay(nextRay, depth + 1) * f_r * dotProduct(nextDir, N) / pdf / RussianRoulette;
// 			}
// 		}

// 		return L_dir + L_indir;
// 	}

// 	return Vector3f(0, 0, 0);
// }
