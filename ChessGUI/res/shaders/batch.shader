#shader vertex
#version 450 core

layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TextCoord;
layout(location = 3) in float a_TextIndex;

uniform mat4 u_MVP;

out vec4 v_Color;
out vec2 v_TextCoord;
out float v_TextIndex;

void main()
{
   v_Color = a_Color;
   v_TextIndex = a_TextIndex;
   v_TextCoord = a_TextCoord;
   gl_Position = u_MVP * a_Position;
};

#shader fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec4 v_Color;
in vec2 v_TextCoord;
in float v_TextIndex;

uniform sampler2D u_Textures[32];

void main()
{ 
   int index = int(v_TextIndex);
   if (index > 0.0)
   {
      o_Color = texture(u_Textures[int(v_TextIndex) - 1], v_TextCoord);
   }
   else
   {
      o_Color = v_Color;
   }
   
};