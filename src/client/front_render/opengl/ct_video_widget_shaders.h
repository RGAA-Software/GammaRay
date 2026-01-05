#ifndef GL_VIDEO_WIDGET_SHADER_H
#define GL_VIDEO_WIDGET_SHADER_H

// #version 330 core
static const char* kMainVertexShader = R"(

    attribute vec3 aPos;
    attribute vec3 aColor;
    attribute vec2 aTex;

    varying vec2 outTex;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {   
        gl_Position = vec4(aPos.x, aPos.y * -1.0, 0.0, 1.0);
        outTex = aTex;
    }

)";

// #version 330 core
static const char* kOperationVertexShader = R"(

    attribute vec3 aPos;
    attribute vec3 aColor;
    attribute vec2 aTex;

    varying vec3 outColor;
    varying vec2 outTex;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {   
        vec4 flipYPos = vec4(aPos.x, aPos.y, 0.0, 1.0);
        vec4 posInWorld = model * flipYPos;
        vec4 posInCamera = view * posInWorld;
        gl_Position = projection * posInCamera;
        gl_Position = vec4(gl_Position.x, gl_Position.y, gl_Position.z, gl_Position.w);

        outColor = aColor;
        outTex = aTex;
    }

)";

//     #version 330 core
static const char* kNV12FragmentShader = R"(

    varying vec3 outColor;
    varying vec2 outTex;

    uniform sampler2D image1;
    uniform sampler2D image2;

    const vec3 delyuv = vec3(-16.0/255.0,-128.0/255.0,-128.0/255.0);
    const vec3 matYUVRGB1 = vec3(1.164, 0.0, 1.596);
    const vec3 matYUVRGB2 = vec3(1.164, -0.391, -0.813);
    const vec3 matYUVRGB3 = vec3(1.164, 2.018, 0.0);

    //out vec4 FragColor;

    uniform int haveImage;

    void main()
    {
        vec4 yColor = texture2D(image1, outTex);
        vec4 uvColor = texture2D(image2, outTex);

        highp vec3 yuv;
        vec3 CurResult;

        yuv.x = yColor.r;
        yuv.y = uvColor.r;
        yuv.z = uvColor.a;

        yuv += delyuv;

        CurResult.x = dot(yuv,matYUVRGB1);
        CurResult.y = dot(yuv,matYUVRGB2);
        CurResult.z = dot(yuv,matYUVRGB3);

        //FragColor = vec4(CurResult.rgb, 1);
        //FragColor = vec4(0.2, 0.3, 0.1, 1.0);

        if(1 == haveImage) {
            gl_FragColor = vec4(CurResult.rgb, 1);
        } else {
            gl_FragColor = vec4(0.2, 0.2, 0.3, 1.0);
        }
    }

)";

// #version 330
static const char* kRGBFragmentShader = R"(

    varying vec3 outColor;
    varying vec2 outTex;

    uniform sampler2D image1;
    //out vec4 FragColor;

    void main()
    {   
        vec4 color = texture2D(image1, outTex);
        gl_FragColor = vec4(color.bgr, 1);
        //FragColor = vec4(color.rgb, 1);
        //FragColor = vec4(1.0, 0.6, 0.3, 1);
    }

)";

//     #version 330
static const char* kRGBAFragmentShader = R"(

    varying vec3 outColor;
    varying vec2 outTex;

    uniform sampler2D image1;
    //out vec4 FragColor;

    void main()
    {   
        vec4 color = texture2D(image1, outTex);
        gl_FragColor = color;
        
        //FragColor = vec4(1.0, 0.6, 0.3, 1);
    }

)";

//    #version 330
static const char* kI420FragmentShader = R"(

    varying vec3 outColor;
    varying vec2 outTex;

    uniform sampler2D imageY;
    uniform sampler2D imageU;
    uniform sampler2D imageV;

    //out vec4 FragColor;

    uniform int haveImage;

    void main()
    {   
        float y, u, v, r, g, b;
        y = texture2D(imageY, outTex).r;
        u = texture2D(imageU, outTex).r;
        v = texture2D(imageV, outTex).r;        
        
        y = 1.164 * (y - 16.0 / 255.0);
        u = u - 128.0 / 255.0;
        v = v - 128.0 / 255.0;

        r = y + 1.596 * v;
        g = y - 0.391 * u - 0.813 * v;
        b = y + 2.018 * u;

        //FragColor = vec4(r, g, b, 1.0); 
        //FragColor = vec4(r, g, b, 1.0);
        //FragColor = vec4(1.0, outColor.r, outTex.g, 1.0);
        //FragColor = vec4(0.2, 0.2, 0.3, 1.0);
        //FragColor = vec4(1.0, 1.0, 1.0, 1.0);

        if(1 == haveImage) {
            gl_FragColor = vec4(r, g, b, 1.0);
        } else {
            gl_FragColor = vec4(0.2, 0.2, 0.3, 1.0);
        }
        //gl_FragColor = vec4(0.2, 0.2, 0.3, 1.0);
    }

)";

//    #version 330
static const char* kI444FragmentShader = R"(

    varying vec3 outColor;
    varying vec2 outTex;

    uniform sampler2D imageY;
    uniform sampler2D imageU;
    uniform sampler2D imageV;

    //out vec4 FragColor;

    uniform int haveImage;

    void main()
    {   
        float y, u, v, r, g, b;
        y = texture2D(imageY, outTex).r;
        u = texture2D(imageU, outTex).r;
        v = texture2D(imageV, outTex).r;        
        
        y = 1.164 * (y - 16.0 / 255.0);
        u = u - 128.0 / 255.0;
        v = v - 128.0 / 255.0;

        r = y + 1.596 * v;
        g = y - 0.391 * u - 0.813 * v;
        b = y + 2.018 * u;

        //FragColor = vec4(r, g, b, 1.0); 
        //FragColor = vec4(r, g, b, 1.0);
        //FragColor = vec4(1.0, outColor.r, outTex.g, 1.0);
        //FragColor = vec4(0.2, 0.2, 0.3, 1.0);
        //FragColor = vec4(1.0, 1.0, 1.0, 1.0);

        if(1 == haveImage) {
            gl_FragColor = vec4(r, g, b, 1.0);
        } else {
            gl_FragColor = vec4(0.2, 0.2, 0.3, 1.0);
        }
    }

)";


#endif // TEX_SHADER_H
