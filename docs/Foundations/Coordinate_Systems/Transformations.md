# Transformations

## Cartesian to Spherical

We want to transform from 3D cartesian coordinates $(x, y, z)$ to spherical coordinates $(r, \theta, \phi)$. From the relationship between spherical coordinates and cartesian coordinates, we have the following equations:

$$
\begin{align}
r &= \sqrt{x^2 + y^2 + z^2} \\
\theta &= \text{atan2}(y, x) \\
\phi &= \sin^{-1} \left( \dfrac{z}{r} \right)
\end{align}
$$

where $\theta \in [-\pi, \pi]$ and $\phi \in \left[ -\frac{\pi}{2}, \frac{\pi}{2} \right]$.

### Implementation

```glsl
vec3 CartesianToSpherical(vec3 cartesian) {
    float x = cartesian.x;
    float y = cartesian.y;
    float z = cartesian.z;

    float r = length(cartesian);
    float theta = (x == 0.0 && y == 0.0) ? 0.0: atan(y, x);
    float phi = asin(clamp(z/max(r, 1e-4), -1.0, 1.0));

    return vec3(r, theta, phi);
}
```

We guard against $r = 0$ with `max` to prevent division by zero. We additionally clamp the expression inside `asin` to prevent NaN propagation from floating-point errors. Similarly, we check if both arguments in `atan` are equal to zero to prevent NaN propagation. Also note that `length` is the same as finding the magnitude of the vector $(x, y, z)$ which is the same thing as $\sqrt{x^2 + y^2 + z^2}$.

## Spherical to Cartesian

We want to transform from spherical coordinates $(r, \theta, \phi)$ to 3D cartesian coordinates $(x, y, z)$. From the relationship between cartesian coordinates and spherical coordinates, we have the following equations:

$$
\begin{align}
x &= r\cos\theta \cdot \cos\phi \\
y &= r\sin\theta \cdot \cos\phi \\
z &= r\sin\phi
\end{align}
$$

### Implementation

```glsl
vec3 SphericalToCartesian(vec3 spherical) {
    float r = spherical.x;
    float theta = spherical.y;
    float phi = spherical.z;

    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);

    float x = r * cosTheta * cosPhi;
    float y = r * sinTheta * cosPhi;
    float z = r * sinPhi;

    return vec3(x, y, z);
}
```

## Spherical to UV

To transform from spherical coordinates $(r, \theta, \phi)$ to 2D UV coordinates $(u, v) \in [0, 1]^2$, we first let $u$ represent the longitude $\theta$ and $v$ represent the latitude $\phi$. To find $u$ and $v$, we must respectively map both $\theta$ and $\phi$ each to range $[0, 1]$. As such,

$$
\begin{align}
u &= \dfrac{\theta}{2\pi} + 0.5 \\
v &= \dfrac{\phi}{\pi} + 0.5
\end{align}
$$

### Implementation

```glsl
vec2 SphericalToUv(vec3 spherical) {
    float theta = spherical.y;
    float phi = spherical.z;

    float u = theta / (2.0*PI) + 0.5;
    float v = phi / (PI) + 0.5;

    // NOTE: Vulkan's UV texture mapping has v flipped
    v = 1.0 - v;

    return vec2(u, v);
}
```

## UV to Spherical

To transform from 2D UV coordinates $(u, v) \in [0, 1]^2$ to spherical coordinates $(r, \theta, \phi)$, we respectively map $u \in [0, 1]$ and $v \in [0, 1]$ to $\theta \in [-\pi, \pi]$ and $\phi \in \left[ -\frac{\pi}{2}, \frac{\pi}{2} \right]$. As such,

$$
\begin{align}
\theta &= 2\pi (u - 0.5) \\
\phi &= \pi (v - 0.5)
\end{align}
$$

### Implementation

```glsl
vec3 UvToSpherical(vec2 uv) {
    float u = uv.x;
    float v = uv.y;

    // Undo the Vulkan v-flip applied in SphericalToUv
    v = 1.0 - v;

    float theta = 2.0 * PI * (u - 0.5);
    float phi = PI * (v - 0.5);

    // UV carries no radial information; assume a unit sphere
    return vec3(1.0, theta, phi);
}
```

## Other Transformations

### Cartesian to UV

```glsl
vec2 CartesianToUv(vec3 cartesian) {
    vec3 spherical = CartesianToSpherical(cartesian);
    vec2 uv = SphericalToUv(spherical);

    return uv;
}
```

### UV to Cartesian

```glsl
vec3 UvToCartesian(vec2 uv) {
    vec3 spherical = UvToSpherical(uv);
    vec3 cartesian = SphericalToCartesian(spherical);

    return cartesian;
}
```

### Directions

3D directions are defined in cartesian coordinates $(x, y, z)$, but they are unique because they must be normalized, meaning they have magnitude $1$. Therefore, in spherical coordinates, the direction vectors have radius $1$ as well. So, we can create functions that use this specific convention of 3D directions.

```glsl
vec3 DirectionToSpherical(vec3 dir) {
    float x = dir.x;
    float y = dir.y;
    float z = dir.z;

    float theta = (x == 0.0 && y == 0.0) ? 0.0 : atan(y, x);
    float phi = asin(clamp(z, -1.0, 1.0));

    return vec3(1.0, theta, phi);
}
```

```glsl
vec3 SphericalToDirection(vec3 spherical) {
    float theta = spherical.y;
    float phi = spherical.z;

    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    float cosPhi = cos(phi);
    float sinPhi = sin(phi);

    float x = cosTheta * cosPhi;
    float y = sinTheta * cosPhi;
    float z = sinPhi;

    return vec3(x, y, z);
}
```

```glsl
vec2 DirectionToUv(vec3 dir) {
    vec3 spherical = DirectionToSpherical(dir);
    vec2 uv = SphericalToUv(spherical);

    return uv;
}
```

```glsl
vec3 UvToDirection(vec2 uv) {
    vec3 spherical = UvToSpherical(uv);
    vec3 dir = SphericalToDirection(spherical);

    return dir;
}
```
