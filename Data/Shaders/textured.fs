#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D TexAlbedo;

void main()
{
    vec4 albedo = texture(TexAlbedo, TexCoords);

#ifdef ALPHA_BLEND
    // pre-multiplied alpha
    albedo.rgb *= albedo.a;

    if( albedo.b > 0.5f ) 
    {
        FragColor = vec4(0, 0, 0, 0);
    }
    else
    {
        FragColor = vec4(albedo.rgb, 1.0f);
    }
#else
    FragColor = vec4(albedo.rgb, 1.0f);
#endif
}
