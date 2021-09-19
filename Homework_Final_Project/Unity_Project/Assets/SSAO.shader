Shader "My/SSAO"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        Cull Off ZWrite Off ZTest Always
        //LOD 100

        CGINCLUDE
        #include "UnityCG.cginc"

        struct v2f_ao {
            float4 vertex : SV_POSITION;
            float2 uv : TEXCOORD0;
            float2 uvr : TEXCOORD1;
        };

        #define INPUT_SAMPLE_COUNT 32

        sampler2D _MainTex;
        float4 _MainTex_ST;
        float4 _MainTex_TexelSize;

        uniform float2 _NoiseScale;
        sampler2D _CameraDepthNormalsTexture;
        float4 _CameraDepthNormalsTexture_ST;
        sampler2D _RandomTexture;
        float _SampleCount;
        float4 _SampleArray[INPUT_SAMPLE_COUNT];
        float4 _Params;

        float4 _BlurRadius;
        float _BilaterFilterFactor;

        sampler2D _AOTex;

        v2f_ao vert(appdata_img v)
        {
            v2f_ao o;
            o.vertex = UnityObjectToClipPos(v.vertex);
            o.uv = TRANSFORM_TEX(v.texcoord, _CameraDepthNormalsTexture);
            o.uvr = v.texcoord.xy * _NoiseScale;
            return o;
        }

        #include "frag_ao.cginc"

        ENDCG

        //Pass 0 : Generate AO 
        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            //#pragma target 3.0

            fixed4 frag(v2f_ao i) : SV_Target
            {
                return frag_ao(i, _SampleCount, _SampleArray);;
            }
            ENDCG
        }

        //Pass 1 : Bilateral Filter Blur
        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            float3 GetNormal(float2 uv)
            {
                float4 cdn = tex2D(_CameraDepthNormalsTexture, uv);
                return DecodeViewNormalStereo(cdn);
            }

            half CompareNormal(float3 normal1, float3 normal2)
            {
                return smoothstep(_BilaterFilterFactor, 1.0, dot(normal1, normal2));
            }

            fixed4 frag(v2f_ao i) : SV_Target
            {
                float2 delta = _MainTex_TexelSize.xy * _BlurRadius.xy;

                float2 uv = i.uv;
                float2 uv0a = i.uv - delta;
                float2 uv0b = i.uv + delta;
                float2 uv1a = i.uv - 2.0 * delta;
                float2 uv1b = i.uv + 2.0 * delta;
                float2 uv2a = i.uv - 3.0 * delta;
                float2 uv2b = i.uv + 3.0 * delta;

                float3 normal = GetNormal(uv);
                float3 normal0a = GetNormal(uv0a);
                float3 normal0b = GetNormal(uv0b);
                float3 normal1a = GetNormal(uv1a);
                float3 normal1b = GetNormal(uv1b);
                float3 normal2a = GetNormal(uv2a);
                float3 normal2b = GetNormal(uv2b);

                fixed4 col = tex2D(_MainTex, uv);
                fixed4 col0a = tex2D(_MainTex, uv0a);
                fixed4 col0b = tex2D(_MainTex, uv0b);
                fixed4 col1a = tex2D(_MainTex, uv1a);
                fixed4 col1b = tex2D(_MainTex, uv1b);
                fixed4 col2a = tex2D(_MainTex, uv2a);
                fixed4 col2b = tex2D(_MainTex, uv2b);

                half w = 0.37004405286;
                half w0a = CompareNormal(normal, normal0a) * 0.31718061674;
                half w0b = CompareNormal(normal, normal0b) * 0.31718061674;
                half w1a = CompareNormal(normal, normal1a) * 0.19823788546;
                half w1b = CompareNormal(normal, normal1b) * 0.19823788546;
                half w2a = CompareNormal(normal, normal2a) * 0.11453744493;
                half w2b = CompareNormal(normal, normal2b) * 0.11453744493;

                half3 result;
                result = w * col.rgb;
                result += w0a * col0a.rgb;
                result += w0b * col0b.rgb;
                result += w1a * col1a.rgb;
                result += w1b * col1b.rgb;
                result += w2a * col2a.rgb;
                result += w2b * col2b.rgb;

                result /= w + w0a + w0b + w1a + w1b + w2a + w2b;
                return fixed4(result, 1.0);
            }
            ENDCG
        }

        //Pass 2 : Composite AO
        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            fixed4 frag(v2f_ao i) : SV_Target
            {
                fixed4 c = tex2D(_MainTex, i.uv);
                fixed4 ao = tex2D(_AOTex, i.uv);
                ao = pow(ao, _Params.w);
                c.rgb *= ao.r;
                return c;
            }
            ENDCG
        }
    }
}
