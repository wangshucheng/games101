
half frag_ao (v2f_ao i, int sampleCount, float4 samples[INPUT_SAMPLE_COUNT])
{
    // (0,1)->(-1,1)
	// read random normal from noise texture
    half3 randN = tex2D (_RandomTexture, i.uvr).xyz * 2.0 - 1.0;    
    
    // read scene depth/normal
    float4 depthnormal = tex2D (_CameraDepthNormalsTexture, i.uv);
    float3 viewNorm;
    float depth;
    DecodeDepthNormal (depthnormal, depth, viewNorm);
    depth *= _ProjectionParams.z;

    // 0. _Params(Radius, DepthBiasValue, 1.0f/OcclusionAttenuation, OcclusionIntensity)
    float scale = _Params.x / depth;
    
    // accumulated occlusion factor
    float occ = 0.0;
    for (int s = 0; s < sampleCount; ++s)
    {
        // 1.��������, ��ÿ����������ԭ�㷨�߷���Ϊ��ת����ת����ĽǶ�. �������²�������ü��䲻����, ������ɢ��. ����Ƶ������ת���ɸ�Ƶ������.
    	// Reflect sample direction around a random vector
        half3 randomDir = reflect(samples[s].xyz, randN);
        

        // 2. �������
        // Make it point to the upper hemisphere
        half flip = (dot(viewNorm,randomDir)<0) ? 1.0 : -1.0;
        randomDir *= -flip;
        // Add a bit of normal to reduce self shadowing
        //randomDir += viewNorm * 0.3;
        

        // 3. ����������ƫ�� ת �ӿ�������ƫ��
        float2 offset = randomDir.xy * scale;
        float randomDepth = depth - (randomDir.z * _Params.x);

		// Sample depth at offset location
        float4 sampleND = tex2D (_CameraDepthNormalsTexture, i.uv + offset);
        float sampleDepth;
        float3 sampleNormal;
        DecodeDepthNormal (sampleND, sampleDepth, sampleNormal);
        sampleDepth *= _ProjectionParams.z;
        float zd = saturate(randomDepth - sampleDepth);
        if (zd > _Params.y) {
        	// This sample occludes, contribute to occlusion
	        occ += pow(1-zd,_Params.z); // sc2
	        //occ += 1.0-saturate(pow(1.0 - zd, 11.0) + zd); // nullsq
        	//occ += 1.0/(1.0+zd*zd*10); // iq
        }        
    }
    occ /= sampleCount;
    return 1-occ;
}

