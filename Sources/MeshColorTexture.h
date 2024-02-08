#pragma once

#include "Mesh.h"
#include <vector>
#include <glm/glm.hpp>
#include "../External/stb_image/stb_image_write.h"

// This case, take uniform resolution and triangles each

void assignUVFace(std::shared_ptr<Mesh> mesh, int textureSize){
  int x = 0, y = 0; // Start from the top left
  int num_rect = floor(mesh->triangleIndices().size()/2.f); // The 2 triangular faces 2i and 2i+1 are going to be combined as a rectangle in the 2D texture
  int r = mesh->resolution();
  int R = pow(2,r) - 1; // face resolution

  int n_rectPerRow = floor(textureSize / (R+5));  // Each rectangle takes R + 5 texel width and R + 2 height
  if (r >= 2) 
    n_rectPerRow = floor(n_rectPerRow / pow(2,r-1))*pow(2,r-1); // make sure that the last texel with color values is divisible by 2^r
  
  int rect = 0;
  for(int f = 0; f < num_rect; f++)
  {
    mesh->addTextureCoordinates(std::vector<glm::vec2>{glm::vec2(x,y+(R+1)), glm::vec2(x,y), glm::vec2(x+R+1, y)}); // face 1
    // Move to face 2
    x += R + 4; // R : edge color + 1 : another vertex + 2: texels for interpolation + 1 : the actual place for vertex 0 of face 2
    mesh->addTextureCoordinates(std::vector<glm::vec2>{glm::vec2(x,y), glm::vec2(x,y+(R+1)), glm::vec2(x-(R+1), y+(R+1))}); // face 2
    
    rect++; 
    if(rect < n_rectPerRow){ // Move to next column
      x++;  
    } else {
      x = 0; 
      y += R+2; // next row of rectangles
      rect = 0;
    }
  }
}

void encodeToPNG(const std::vector<std::vector<glm::vec4>> texture2d, const char* filename){ // width = height
  int size = static_cast<int>(texture2d[0].size());
  unsigned char * textureData = new unsigned char[4*size*size];
   // Fill the buffer with data from the glm::vec4 values
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            glm::vec4 color = texture2d[y][x];
            int index = (y * size + x) * 4;
            textureData[index + 0] = static_cast<unsigned char>(color.r * 255.0f);
            textureData[index + 1] = static_cast<unsigned char>(color.g * 255.0f);
            textureData[index + 2] = static_cast<unsigned char>(color.b * 255.0f);
            textureData[index + 3] = static_cast<unsigned char>(color.a * 255.0f);
        }
    }
    stbi_write_png(filename,size,size,4,textureData,size*4);
    delete[] textureData;
}

// 3.2 - 3.3 Generate different mipmap levels with 4D coordinates
void generateMipmaps(std::shared_ptr<Mesh> mesh, std::vector<std::vector<glm::vec4>> mipmap_level0, int texelSize)//, std::vector<std::vector<glm::vec4>> texel2d)
{
  // calculate u_s, u_delta for each rectangular face
  int r = mesh->resolution();
  int power = pow(2,r);
  int R = power -1;
  int n_face = floor(mesh->triangleIndices().size()/2);

  std::vector<std::vector<glm::vec2>> textureCoords = mesh->textureCoordinates();
  std::vector<std::vector<glm::vec2>> u_s,u_l;  // texture coordinates for each face
  std::vector<glm::vec2>  u_delta;

  /*for (int f = 0; f < n_face; f++)
  {
    glm::vec2 u_delta_face = glm::mod(textureCoords[2*f][1], glm::vec2(power)); // Since i'm not using texture coordinates anyway, I dont need to add the offset (0.5,0.5) 
    u_delta.push_back(u_delta_face); 
    u_s.push_back({textureCoords[2*f][1]-u_delta_face+glm::vec2(0,power), textureCoords[2*f][1]-u_delta_face, textureCoords[2*f][1]-u_delta_face+ glm::vec2(power,0), textureCoords[2*f][1]-u_delta_face + glm::vec2(power,power)});
  }*/
  glm::vec2 us(0,0);
  glm::vec2 ud(0,0);
  int n_rectPerRow = floor(texelSize / (R+5));  // Each rectangle takes R + 5 texel width and R + 2 height
  if (r >= 2) 
    n_rectPerRow = floor(n_rectPerRow / pow(2,r-1))*pow(2,r-1); // make sure that the last texel with color values is divisible by 2^r
  
  for(int row = 0;row<floor(n_face/n_rectPerRow);row++){
     //glm::vec2 ud = glm::vec2(0,(R+1)*row);
    //glm::vec2 us = glm::vec2(0,(R+1)*row);
    for(int col = 0; col < n_rectPerRow; col++){
      if (u_s.size() == n_face)
        break;
      u_s.push_back({us-ud+glm::vec2(0,R+1),us-ud,us-ud+glm::vec2(R+1,0),us-ud+glm::vec2(R+1,R+1)});
      u_delta.push_back(ud);
      int usx = ud.x+R+1;
      int udx = usx+1;
      ud = glm::vec2(udx,ud.y);
      us = glm::vec2(usx,us.y);
    } 
    ud += glm::vec2(0,1) + glm::vec2(-ud.x,0);
    us = glm::vec2(0,(R+2)*(row+1));
  }
  for (int level = 0; level <= r; level++)
  {
    float coef = pow(2.f,level);
    int R_l = pow(2.f, r - level) -1; // resolution for the mipmap level
    for(int f = 0; f < n_face; f++)
      u_l.push_back({(u_s[f][0]/coef)+u_delta[f],(u_s[f][1]/coef)+u_delta[f],(u_s[f][1]/coef)+u_delta[f] + glm::vec2(power/coef,0),(u_s[f][2]/coef)+u_delta[f]+glm::vec2(3,0),(u_s[f][3]/coef)+u_delta[f] + glm::vec2(3,0),(u_s[f][3]/coef)+u_delta[f] + glm::vec2(3-power/coef,0)}); 
      //u_l.push_back({(u_s[f][0]/coef)+u_delta[f],(u_s[f][1]/coef)+u_delta[f],(u_s[f][2]/coef)+u_delta[f],(u_s[f][3]/coef)+u_delta[f]}); 
    // store 6 vertices instead of 4 for u_l
    
    std::vector<std::vector<glm::vec4>> mipmap_l(texelSize, std::vector<glm::vec4> (texelSize, glm::vec4{1.f,1.f,1.f,0.f}));
    
    for(int f = 0; f < n_face; f++)
    {
      for(int v = 0; v < 6; v++)
        mipmap_l[u_l[f][v].y][u_l[f][v].x] = glm::vec4(mesh->meshColors().vertexColors()[mesh->triangleIndices()[2*f + floor(v/3)][v%3]],1.0f);
      int edge_dist = 1;
      while(edge_dist <= R_l)
      {
        // for the edge color, I store their values on counter clock-wise, from 1->2, 2->3, 3->1
        mipmap_l[u_l[f][0].y-edge_dist][u_l[f][0].x] = glm::vec4(mesh->meshColors().edgeColors()[2*f*(3*0+edge_dist)],1.0f);
        mipmap_l[u_l[f][1].y][u_l[f][1].x+edge_dist] = glm::vec4(mesh->meshColors().edgeColors()[2*f*(3*1+edge_dist)],1.0f);
        mipmap_l[u_l[f][2].y+edge_dist][u_l[f][2].x-edge_dist] = glm::vec4(mesh->meshColors().edgeColors()[2*f*(3*2+edge_dist)],1.0f);

        mipmap_l[u_l[f][3].y+edge_dist][u_l[f][3].x] = glm::vec4(mesh->meshColors().edgeColors()[(2*f+1)*(3*0+edge_dist)],1.0f);
        mipmap_l[u_l[f][4].y][u_l[f][4].x-edge_dist] = glm::vec4(mesh->meshColors().edgeColors()[(2*f+1)*(3*1+edge_dist)],1.0f);
        mipmap_l[u_l[f][5].y-edge_dist][u_l[f][5].x+edge_dist] = glm::vec4(mesh->meshColors().edgeColors()[(2*f+1)*(3*2+edge_dist)],1.0f);
      
        int faceColorId=f*(pow(2,r)-1);
        for (int i = 1; i < edge_dist; i++)
        {
          mipmap_l[u_l[f][0].y-edge_dist][u_l[f][0].x+i] = glm::vec4(mesh->meshColors().faceColors()[2*faceColorId+edge_dist],1.0f);
          mipmap_l[u_l[f][3].y+edge_dist][u_l[f][3].x-i] = glm::vec4(mesh->meshColors().faceColors()[(2*faceColorId+1)+edge_dist],1.0f);
        }
        edge_dist++;
      }
    }
        
    std::string filename = "mipmap_level_" + std::to_string(level) + ".png";
    encodeToPNG(mipmap_l, filename.c_str());
    u_l.clear();
  
  }
}

void generate2DTextureLayout(std::shared_ptr<Mesh> mesh, int textureSize, const char* filename){
  assignUVFace(mesh, textureSize);
  std::vector<std::vector<glm::vec4>> texel2d(textureSize, std::vector<glm::vec4>(textureSize, glm::vec4{1.f, 1.f, 1.f,0.f}));
  int r = mesh->resolution(); // small r
  int R = pow(2,r) - 1;

  int faceColorIndex = 0;
  for (int f = 0; f < mesh->triangleIndices().size(); f++)
  {
    std::vector<glm::vec2> ft = mesh->textureCoordinates()[f]; // face texture
    for (int v = 0; v < 3; v++)
      texel2d[ft[v].y][ft[v].x] = glm::vec4(mesh->meshColors().vertexColors()[mesh->triangleIndices()[f][v]],1.0f);

    int edge_dist = 1; // texel distance from vertex 1 
    int edgeColorIndex = f*R;
    if (f % 2 == 0){ 
      while(edge_dist <= R){
        texel2d[ft[0].y - edge_dist][ft[0].x] = glm::vec4(mesh->meshColors().edgeColors()[f*(3*0 + edge_dist)],1.f); // edge 1 : vertical
        texel2d[ft[1].y][ft[1].x + edge_dist] = glm::vec4(mesh->meshColors().edgeColors()[f*(3*1 + edge_dist)],1.f); // edge 2 : horizontal
        texel2d[ft[2].y + edge_dist][ft[2].x - edge_dist] = glm::vec4(mesh->meshColors().edgeColors()[f*(3*2 + edge_dist)],1.f); // edge 3 : diagonal
        for (int i = 1; i < edge_dist; i++)
          if (faceColorIndex < mesh->meshColors().faceColors().size()){
            texel2d[ft[0].y-edge_dist][ft[0].x + i] = glm::vec4(mesh->meshColors().faceColors()[faceColorIndex],1.f);
            faceColorIndex+=1;
          }
        edge_dist+=1;
      }
      // Interpolate C3 
      int x_c3 = ft[0].x + 1;
      int y_c3 = ft[0].y;

      while (y_c3 > ft[2].y){
        glm::vec4 c3 = texel2d[y_c3][x_c3 - 1] + texel2d[y_c3-1][x_c3] - texel2d[y_c3-1][x_c3 - 1];
        glm::vec3 delta(0.0f);
        if (c3.r < 0 || c3.g < 0 || c3.b < 0)
            delta = -glm::vec3(c3.r,c3.g,c3.b)/3.0f;
        else if (c3.r > 1 || c3.g > 1 || c3.b > 1)
            delta = glm::vec3(1-c3.r,1-c3.g,1-c3.b)/3.0f;
        texel2d[y_c3][x_c3 - 1] += glm::vec4(delta,0.0f); // Update the mesh color data
        texel2d[y_c3-1][x_c3] += glm::vec4(delta,0.0f); 
        texel2d[y_c3-1][x_c3-1] -= glm::vec4(delta,0.0f);

        texel2d[y_c3][x_c3] = c3;
        x_c3++;
        y_c3--;
      }
    }
    else{
      while(edge_dist <= R){
        texel2d[ft[0].y + edge_dist][ft[0].x] = glm::vec4(mesh->meshColors().edgeColors()[f*(3*0 + edge_dist)],1.f); // edge 1 : vertical
        texel2d[ft[1].y][ft[1].x - edge_dist] = glm::vec4(mesh->meshColors().edgeColors()[f*(3*1 + edge_dist)],1.f); // edge 2 : horizontal
        texel2d[ft[2].y - edge_dist][ft[2].x + edge_dist] = glm::vec4(mesh->meshColors().edgeColors()[f*(3*2 + edge_dist)],1.f); // edge 3 : diagonal
        for (int i = 1; i < edge_dist; i++)
          if (faceColorIndex < mesh->meshColors().faceColors().size()){
            texel2d[ft[0].y+edge_dist][ft[0].x - i] = glm::vec4(mesh->meshColors().faceColors()[faceColorIndex],1.f);
            faceColorIndex+=1;
          }
        edge_dist+=1;
      }
      
      // Interpolate C3 
      int x_c3 = ft[0].x - 1, y_c3 = ft[0].y;
      
      while (y_c3 < ft[2].y){
        glm::vec4 c3 = texel2d[y_c3][x_c3+1] + texel2d[y_c3+1][x_c3] - texel2d[y_c3+1][x_c3+1];
        glm::vec3 delta(0.0f);
        if (c3.r < 0 || c3.g < 0 || c3.b < 0)
            delta = -glm::vec3(c3.r,c3.g,c3.b)/3.0f;
        else if (c3.r > 1 || c3.g > 1 || c3.b > 1)
            delta = glm::vec3(1-c3.r,1-c3.g,1-c3.b)/3.0f;
        texel2d[y_c3][x_c3 + 1] += glm::vec4(delta,0.0f); // Update the mesh color data
        texel2d[y_c3+1][x_c3] += glm::vec4(delta,0.0f); 
        texel2d[y_c3+1][x_c3 + 1] -= glm::vec4(delta,0.0f);

        texel2d[y_c3][x_c3] = c3;//texel2d[y_c3][x_c3 + 1] + texel2d[y_c3+1][x_c3] - texel2d[y_c3+1][x_c3 + 1];
        x_c3--;
        y_c3++;
      }
    }
  }
  encodeToPNG(texel2d, filename);
  generateMipmaps(mesh,texel2d,textureSize);
}

