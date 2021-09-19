using System.Collections;
using System.Collections.Generic;
using UnityEngine;

[ExecuteInEditMode]
[RequireComponent(typeof(Camera))]
public class SSAO : MonoBehaviour
{
    public Texture2D RandomTexture;

    [Range(0, 2)]
    public int DownSample = 0;

    [Range(0f, 4.0f)]
    public float OcclusionIntensity = 1.5f;
    [Range(0.01f, 1.0f)]
    public float OcclusionAttenuation = 1.0f;
    [Range(4, 32)]
    public int SampleCount = 16;
    [Range(0.01f, 0.5f)]
    public float Radius = 0.4f;
    [Range(0, 0.003f)]
    public float DepthBiasValue = 0.002f;

    [Range(1, 4)]
    public int BlurRadius = 1;
    [Range(0, 0.5f)]
    public float BilaterFilterStrength = 0.2f;

    public bool OnlyShowAO = false;

    public enum SSAOPassName
    {
        GenerateAO = 0,
        BilateralFilter = 1,
        Composite = 2,
    }

    List<Vector4> sampleList = new List<Vector4>();

    Material m_material;
    Material Mat
    {
        get
        {
            if (m_material == null)
            {
                Shader shader = Shader.Find("My/SSAO");
                m_material = new Material(shader);
                m_material.hideFlags = HideFlags.HideAndDontSave;
                m_material.SetTexture("_RandomTexture", RandomTexture);
            }
            return m_material;
        }
    }

    Camera m_camera;
    Camera Cam
    {
        get
        {
            if (m_camera == null)
            {
                m_camera = GetComponent<Camera>();
            }
            return m_camera;
        }
    }

    void OnEnable()
    {
        Cam.depthTextureMode |= DepthTextureMode.DepthNormals;
    }
    private void OnDisable()
    {
        Cam.depthTextureMode &= ~DepthTextureMode.DepthNormals;
    }

    void Start()
    {

    }

    private void OnRenderImage(RenderTexture source, RenderTexture destination)
    {
        GenerateAOSample();

        var aoRT = RenderTexture.GetTemporary(source.width >> DownSample, source.height >> DownSample, 0);

        Mat.SetFloat("_SampleCount", SampleCount);
        Mat.SetVectorArray("_SampleArray", sampleList.ToArray());

        int noiseWidth, noiseHeight;
        if (RandomTexture)
        {
            noiseWidth = RandomTexture.width;
            noiseHeight = RandomTexture.height;
        }
        else
        {
            noiseWidth = 1; noiseHeight = 1;
        }
        Mat.SetVector("_NoiseScale", new Vector3((float)aoRT.width / noiseWidth, (float)aoRT.height / noiseHeight, 0.0f));
        Mat.SetVector("_Params", new Vector4(Radius, DepthBiasValue, 1.0f / OcclusionAttenuation, OcclusionIntensity));
        Graphics.Blit(source, aoRT, Mat, (int)SSAOPassName.GenerateAO);

        var blurRT = RenderTexture.GetTemporary(source.width >> DownSample, source.height >> DownSample, 0);
        Mat.SetFloat("_BilaterFilterFactor", 1.0f - BilaterFilterStrength);

        Mat.SetVector("_BlurRadius", new Vector4(BlurRadius, 0, 0, 0));
        Graphics.Blit(aoRT, blurRT, Mat, (int)SSAOPassName.BilateralFilter);

        Mat.SetVector("_BlurRadius", new Vector4(0, BlurRadius, 0, 0));
        if (OnlyShowAO)
        {
            Graphics.Blit(blurRT, destination, Mat, (int)SSAOPassName.BilateralFilter);
        }
        else
        {
            Graphics.Blit(blurRT, aoRT, Mat, (int)SSAOPassName.BilateralFilter);
            Mat.SetTexture("_AOTex", aoRT);
            Graphics.Blit(source, destination, Mat, (int)SSAOPassName.Composite);
        }

        RenderTexture.ReleaseTemporary(aoRT);
        RenderTexture.ReleaseTemporary(blurRT);
    }

    private void GenerateAOSample()
    {
        if (SampleCount == sampleList.Count)
            return;
        sampleList.Clear();
        for (int i = 0; i < SampleCount; i++)
        {
            var vec = new Vector4(Random.Range(-1.0f, 1.0f), Random.Range(-1.0f, 1.0f), Random.Range(0, 1.0f), 1.0f);
            vec.Normalize();
            var scale = (float)i / SampleCount;
            //使分布符合二次方程的曲线
            scale = Mathf.Lerp(0.01f, 1.0f, scale * scale);
            vec *= scale;
            sampleList.Add(vec);
        }
    }
}
