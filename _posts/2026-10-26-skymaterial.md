---
layout: post
title: With Qt 6.12, the SkyMaterial is the limit
markdown: kramdown
highlighter: rouge
---

NOTE: This is a copy-pasted version of my [blog post](https://www.qt.io/blog/with-qt-6.12-the-skymaterial-is-the-limit) on the Qt blog.

[Qt Quick 3D](https://doc.qt.io/qt-6.12/qtquick3d-index.html) 6.12 introduces a
new [SkyMaterial](https://doc.qt.io/qt-6.12/qml-qtquick3d-skymaterial.html) QML
type, and in this post we will explain what it is, how to use it and why you'd
want to.

SkyMaterial provides a way of writing custom fragment shader code to render a
scene's sky, as well as computing image-based lighting (IBL) from it. This means
the same sky acts as both the visible background and a light source for the
scene. With it you can create a dynamic sky with a moving sun, day/night cycle,
clouds, stars etc.

QML properties set on the SkyMaterial type are automatically made available in
the custom shader, and whenever a property is updated, the SkyMaterial
re-renders itself. This is what makes it possible to have a dynamic sky. Since
the rendering is happening on the GPU, the performance cost depends on the
complexity of the shader and how often it is updated. It is possible to tweak
certain quality and performance properties to fit your performance budget.

To use SkyMaterial in your 3D scene you need to point the
[SceneEnvironment::skyMaterial](https://doc.qt.io/qt-6.12/qml-qtquick3d-sceneenvironment.html#skyMaterial-prop)
property to your SkyMaterial object, and set
[SceneEnvironment::backgroundMode](https://doc.qt.io/qt-6.12/qml-qtquick3d-sceneenvironment.html#backgroundMode-prop)
to `SceneEnvironment.SkyMaterial`.

# A first sky

Let's look at a minimal example of SkyMaterial in action:

```qml
import QtQuick
import QtQuick3D

Window {
    width: 800
    height: 600
    visible: true

    View3D {
        anchors.fill: parent
        environment: SceneEnvironment {
            backgroundMode: SceneEnvironment.SkyMaterial
            skyMaterial: SkyMaterial {
                fragmentShaderCode: `
                    void MAIN()
                    {
                        vec3 dir = normalize(qt_eyeDir);
                        // Map from [-1, 1] to [0, 1]
                        FRAGCOLOR = vec4(dir * 0.5 + 0.5, 1.0);
                    }
                `
            }
        }

        camera: PerspectiveCamera {
            position: Qt.vector3d(0, 100, 400)
        }

        Model {
            source: "#Cube"
            materials: PrincipledMaterial {}
        }
    }
}
```

And this is how it looks:

![A cube against a sky colored by view direction](/assets/img/QtFramework-3D-SkyMaterialViewDirectionSky-1280x720.png "A cube against a sky colored by view direction")

What we have here is a 3D scene with a cube in the middle and a SkyMaterial with
the
[fragmentShaderCode](https://doc.qt.io/qt-6.12/qml-qtquick3d-skymaterial.html#fragmentShaderCode-prop)
set to an inlined custom fragment shader (use
[fragmentShader](https://doc.qt.io/qt-6.12/qml-qtquick3d-skymaterial.html#fragmentShader-prop)
to load the shader from a file). The
[skyboxMode](https://doc.qt.io/qt-6.12/qml-qtquick3d-skymaterial.html#skyboxMode-prop)
property is set to `SkyMaterial.Cubemap` by default which means the sky is
rendered into a cubemap with the shader running for each of its six faces.
Likewise, the
[enableIBL](https://doc.qt.io/qt-6.12/qml-qtquick3d-skymaterial.html#enableIBL-prop)
property is set to `true` by default, which also filters the cubemap into
radiance and irradiance maps used for IBL. The irradiance map is what lights the
cube, so each face takes on the color of the part of the sky it faces.

Let's look at the shader. Firstly, it must define a `MAIN()` entry point, which
is executed for each fragment generated while rendering the cubemap. The shader
has access to the built-in variable `qt_eyeDir`, which provides the
non-normalized direction vector from the center of the cubemap towards the
fragment being rendered. The final pixel color must be written to `FRAGCOLOR`,
which is the output variable of the fragment shader.

In the example above we color the `FRAGCOLOR` rgb components from the sampled
direction just to verify that the shader is working.

# Properties become uniforms

As mentioned earlier, compatible properties set on SkyMaterial will
automatically be available in the shader. So let's add a property and tweak our
shader so it reacts to it:

```qml
skyMaterial: SkyMaterial {
    property real angle: 0
    NumberAnimation on angle {
        from: 0
        to: 360
        duration: 6000
        loops: Animation.Infinite
    }

    fragmentShaderCode: `
        void MAIN()
        {
            vec3 dir = normalize(qt_eyeDir);
            // Rotate the view direction around the Y axis by angle degrees.
            float a = radians(angle);
            float c = cos(a);
            float s = sin(a);
            vec3 rotated = vec3(c * dir.x + s * dir.z,
                                dir.y,
                                -s * dir.x + c * dir.z);
            vec3 color = rotated * 0.5 + 0.5;
            FRAGCOLOR = vec4(color, 1.0);
        }
    `
}
```

What we've added here is a `real` property `angle` and a NumberAnimation that
acts on it. What this will do is rotate the view direction around the Y axis by
`angle` degrees, so the sky makes a full rotation every six seconds. It looks
like this:

<video autoplay loop muted playsinline controls style="width:100%; height:auto;">
  <source src="{{ '/assets/vids/QtFramework-3D-SkyMaterialRotatingSkyColors-1280x720.mp4' | relative_url }}" type="video/mp4">
  Your browser does not support the video tag.
</video>

Keep in mind that with every change of `angle` the sky and the IBL cubemap get
re-rendered. The higher the resolution of the cubemap, the longer it takes, so
tweaking the size using the
[radianceMapSize](https://doc.qt.io/qt-6.12/qml-qtquick3d-skymaterial.html#radianceMapSize-prop)
property can be useful.

If the sky changes every frame like here and you don't need IBL, you can set
[enableIBL](https://doc.qt.io/qt-6.12/qml-qtquick3d-skymaterial.html#enableIBL-prop)
to `false` and
[skyboxMode](https://doc.qt.io/qt-6.12/qml-qtquick3d-skymaterial.html#skyboxMode-prop)
to `SkyMaterial.ScreenSpaceFull`, `SkyMaterial.ScreenSpaceHalf` or
`SkyMaterial.ScreenSpaceQuarter`. Then no cubemap or IBL is rendered, and the
sky shader runs directly on screen instead, at full, half or quarter
resolution. Note that this happens every frame, even when nothing changes, so
for a sky that stays mostly static the default `SkyMaterial.Cubemap` mode is cheaper. It is
rendered once into a cubemap and then just sampled from it.

Besides `real`, SkyMaterial supports the QML types that have a natural GLSL
counterpart, such as `int`, `bool`, `color`, vectors, matrices and textures. See
the [SkyMaterial
documentation](https://doc.qt.io/qt-6.12/qml-qtquick3d-skymaterial.html) for the
full list of types and what they map to in GLSL.

# A procedural sky

With these types of properties we can implement more advanced shaders, so let's
do that. This is how our new SkyMaterial object looks:

```qml
skyMaterial: SkyMaterial {
    iblSampleCount: 256

    property color skyTopColor: "#A5D6F1"
    property color skyHorizonColor: "#D6EAFA"
    property real skyCurve: 0.09
    property real skyEnergy: 1.0
    property color groundBottomColor: "#282F36"
    property color groundHorizonColor: "#6C655F"
    property real groundCurve: 0.02
    property real groundEnergy: 1.0
    property color sunColor: "#ffffff"
    property real sunLatitude: 35.0
    property real sunLongitude: 0.0
    property real sunAngleMin: 1.0
    property real sunAngleMax: 100.0
    property real sunCurve: 0.05
    property real sunEnergy: 1.0

    readonly property vector3d sunDirection: {
        const latRad = sunLatitude * Math.PI / 180.0
        const lonRad = sunLongitude * Math.PI / 180.0
        return Qt.vector3d(
            -Math.cos(lonRad) * Math.cos(latRad),
             Math.sin(latRad),
            -Math.sin(lonRad) * Math.cos(latRad)
        )
    }

    fragmentShaderCode: `
        const float PI = 3.14159265359;
        const float HALF_PI = PI * 0.5;

        float ease(float x, float c)
        {
            x = clamp(x, 0.0, 1.0);
            if (c > 0.0) {
                if (c < 1.0)
                    return 1.0 - pow(1.0 - x, 1.0 / c);
                return pow(x, c);
            }
            if (c < 0.0) {
                if (x < 0.5)
                    return pow(x * 2.0, -c) * 0.5;
                return (1.0 - pow(1.0 - (x - 0.5) * 2.0, -c)) * 0.5 + 0.5;
            }
            return 0.0;
        }

        void MAIN()
        {
            vec3 direction = normalize(qt_eyeDir);
            float vAngle = acos(clamp(direction.y, -1.0, 1.0));

            vec3 color;

            if (direction.y < 0.0) {
                float t = ease((vAngle - HALF_PI) / HALF_PI, groundCurve);
                color = mix(groundHorizonColor.rgb, groundBottomColor.rgb, t) * groundEnergy;
            } else {
                float t = ease(1.0 - vAngle / HALF_PI, skyCurve);
                color = mix(skyHorizonColor.rgb, skyTopColor.rgb, t) * skyEnergy;

                float sunAngle = degrees(acos(clamp(dot(normalize(sunDirection), direction), -1.0, 1.0)));
                vec3 sunRadiance = sunColor.rgb * sunEnergy;

                if (sunAngle < sunAngleMin) {
                    color = sunRadiance;
                } else if (sunAngle < sunAngleMax) {
                    float tSun = ease(
                        (sunAngle - sunAngleMin) / (sunAngleMax - sunAngleMin),
                        sunCurve
                    );
                    color = mix(sunRadiance, color, tSun);
                }
            }

            FRAGCOLOR = vec4(color, 1.0);
        }
    `
}
```

The scene now looks like this:

![A cube against a procedural sky with a sun](/assets/img/QtFramework-3D-SkyMaterialProceduralSkyWithSun-1280x720.png "A cube against a procedural sky with a sun")

The properties at the top control the colors of the sky and ground, and the position of the sun. The shader has access to all of them, and the sky is re-rendered whenever any of them change. The [iblSampleCount](https://doc.qt.io/qt-6.12/qml-qtquick3d-skymaterial.html#iblSampleCount-prop) property controls the number of samples used in the IBL computation, and by extension the visual quality of the generated radiance map. Since the sky is static here and only computed once we can increase it to get better visual fidelity.

# Try it out

With SkyMaterial as the base you can go much further, and for an example of
that you can achieve, check out the official [Qt Quick 3D - Sky Material
Example](https://doc.qt.io/qt-6.12/qtquick3d-skymaterial-example.html) that
comes with Qt 6.12. The example implements physically based atmospheric
scattering with ray-marched clouds. It looks like this:

<video autoplay loop muted playsinline controls style="width:100%; height:auto;">
  <source src="{{ '/assets/vids/QtFramework-3D-SkyMaterialExampleClouds-1280x720.mp4' | relative_url }}" type="video/mp4">
  Your browser does not support the video tag.
</video>

That's it for this introduction to SkyMaterial in Qt 6.12. I hope it inspires
you to create your own skies, and I look forward to seeing what you make!
