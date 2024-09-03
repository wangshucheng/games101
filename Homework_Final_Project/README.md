Games101大作业
---

## 题目

屏幕空间的环境光遮蔽SSAO

## 开发环境

Win10+Unity2020+VS2019

## 实现方式

1. 计算遮蔽因子作为环境光ambient的系数
2. 作为后处理，计算ao图与RT叠加

> 本次实现采用第二种方式

## 实现过程

1. 计算屏幕空间的每一个像素在三维空间的坐标p
2. 以p为圆心，R为半径的半球（使用法线缓冲的方向）随机若干采样点
3. 计算每个采样点在深度缓存上的投影点 
4. 使用投影点跟 p 点深度值差异计算遮蔽大小
5. 计算所有采样点的平均遮蔽
6. 使用低通滤波去掉高频噪声（高斯滤波或双边滤波，本次实现采用双边滤波）
7. 输出ao图

## 可视化参数

- DownSample：降分辨率，0/1/2
- OcclusionIntensity：AO强度，越大越暗，0.~4.
- OcclusionAttenuation：遮蔽距离衰减，0.01~1.
- SampleCount：采样次数，4~32
- Radius：半径，0.01~0.5
- DepthBiasValue：深度偏差值，0.~0.003
- BlurRadius：模糊半径，1/2/3/4
- BilaterFilterStrength：双边滤波强度，0~0.5

## 关键代码

1. SSAO.cs，后处理及参数控制代码
2. SSAO.shader，调用计算遮蔽/模糊/叠加AO的shader代码
3. frag_ao.cginc，ao核心cg代码

## 实现结果

rock_no_ao
![](E:\workspace\course\GAMES101\GAMES101-HOMEWORK2021\Homework_Final_Project\images\rock_no_ao.png)

rock_only_ao
![](E:\workspace\course\GAMES101\GAMES101-HOMEWORK2021\Homework_Final_Project\images\rock_only_ao.png)

rock_with_ao
![](E:\workspace\course\GAMES101\GAMES101-HOMEWORK2021\Homework_Final_Project\images\rock_with_ao.png)

role_no_ao
![](E:\workspace\course\GAMES101\GAMES101-HOMEWORK2021\Homework_Final_Project\images\role_no_ao.png)

role_only_ao
![](E:\workspace\course\GAMES101\GAMES101-HOMEWORK2021\Homework_Final_Project\images\role_only_ao.png)

role_with_ao
![](E:\workspace\course\GAMES101\GAMES101-HOMEWORK2021\Homework_Final_Project\images\role_with_ao.png)

