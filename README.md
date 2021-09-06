# games101_2021
games101





## Windows下修改

### hw5

[作业5编译选项 -fsanitize=undefined 必须吗？ – 计算机图形学与混合现实研讨会 (games-cn.org)](http://games-cn.org/forums/topic/作业5编译选项-fsanitizeundefined-必须吗？/)

[UndefinedBehaviorSanitizer — Clang 13 documentation (llvm.org)](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html)

CMakeLists.txt 去掉 -fsanitize=undefined



### hw7

```
inline float get_random_float()
{
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(0.f, 1.f); // distribution in range [1, 6]

    return dist(rng);
}
```

win10+MinGW+gcc 

std::random_device dev;

运行时报错，原因未知

重新实现

```
inline float get_random_float()
{
    return rand()%10000/(float)10000;
}
```


## Reference
https://www.cnblogs.com/coolwx/category/1921036.html
