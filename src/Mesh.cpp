#include <memory>
#include <iostream>

#include "transform.h"
#include "globals.h"
#include "stb_image.h"

void Mesh::destroy_buffers(){
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &NBO);
	glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &UVBO);
    glDeleteBuffers(1, &ambiant_bo);
    glDeleteBuffers(1, &diffuse_bo);
    glDeleteBuffers(1, &specular_bo);
    glDeleteBuffers(1, &mra_bo);

    glDeleteVertexArrays(1, &VAO_wireframe);
    glDeleteBuffers(1, &VBO_wireframe);
    glDeleteBuffers(1, &EBO_wireframe);
}

void Mesh::openglInit()
{
    // ----------------- MESH ---------------- //

    // Transfer mesh data to GPU
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &NBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1,&UVBO);
    glGenBuffers(1,&ambiant_bo);
    glGenBuffers(1,&diffuse_bo);
    glGenBuffers(1,&specular_bo);
    glGenBuffers(1,&mra_bo);
    glGenBuffers(1,&useTexture_bo);

    // Bind VAO
    glBindVertexArray(VAO);

    // Bind VBO and copy vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    // Set vertex attribute pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Bind EBO and copy index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    // Bind normals to layout location 1
    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1); // This allows usage of layout location 1 in the vertex shader
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Bind UV to layout location 2
    glBindBuffer(GL_ARRAY_BUFFER, UVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * uv.size(), uv.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2); // This allows usage of layout location 2 in the vertex shader
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // ----------------- MATERIAL ---------------- //

    send_material_to_shaders();


    // ----------------- TEXTURES ---------------- //

    diffuse_texture_LOC = glGetUniformLocation(shaderProgram_main, "diffuse_texture");
    float_texture_LOC = glGetUniformLocation(shaderProgram_main, "float_texture");

    glUseProgram(shaderProgram_Flat);
    glUniform1i(diffuse_texture_LOC, 0);
    glUniform1i(float_texture_LOC,  1);

    glUseProgram(shaderProgram_main);
    glUniform1i(diffuse_texture_LOC, 0);
    glUniform1i(float_texture_LOC,  1);

    glGenTextures(1, &diffuse_texture_id);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, diffuse_texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 material.diffuse_texture.width,
                 material.diffuse_texture.height,
                 0,
                 GL_RGB,
                 GL_UNSIGNED_BYTE,
                 material.diffuse_texture.data.data()
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    glGenTextures(1, &float_texture_id);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, float_texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 material.float_texture.width,
                 material.float_texture.height,
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 gpuOutputImg.data()
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    // ------------------------ WireFrame VAO -------------------------

    createWireframeIndicies();

    glGenVertexArrays(1, &VAO_wireframe);
    glGenBuffers(1, &VBO_wireframe);
    glGenBuffers(1, &EBO_wireframe);


    glBindVertexArray(VAO_wireframe);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_wireframe);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_wireframe);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * wireframeLineIndicies.size(), wireframeLineIndicies.data(), GL_STATIC_DRAW);
}

void Mesh::change_texture(FloatTexture tex)
{
    material.float_texture.width = tex.width;
    material.float_texture.height = tex.height;

    material.float_texture.data = tex.data;

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, diffuse_texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB,
                 material.float_texture.width,
                 material.float_texture.height,
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 tex.data.data()
    );
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Mesh::send_material_to_shaders()
{
    if(textureBufferAlreadyCreated){
        updateMaterial();
        return;
    }

    // DIFFUSE
    std::vector<glm::vec3> diffuse_vec;
    for(int i = 0; i < vertices.size(); ++i){
        diffuse_vec.push_back(material.diffuse_material);
    }
    glBindBuffer(GL_ARRAY_BUFFER, diffuse_bo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), diffuse_vec.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    // MRA
    glm::vec3 mra = material.getMRA();

    std::vector<glm::vec3> mra_vec;
    for(int i = 0; i < vertices.size(); ++i){
        mra_vec.push_back(mra);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mra_bo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), mra_vec.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // Send useTexture, isEmissive and isTransparent parameters
    std::vector<glm::vec3> useT_vec;
    for(int i = 0; i < vertices.size(); ++i){
        useT_vec.push_back(vec3(material.useTexture, material.isEmissive, material.isTransparent));
    }
    glBindBuffer(GL_ARRAY_BUFFER, useTexture_bo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), useT_vec.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    textureBufferAlreadyCreated = true;

    //////////////////////////// SEND UPDATED MATERIALS TO KERNEL.CL ////////////////////////

}

void Mesh::updateMaterial(){
    // DIFFUSE
    std::vector<glm::vec3> diffuse_vec;
    for(int i = 0; i < vertices.size(); ++i){
        diffuse_vec.push_back(material.diffuse_material);
    }
    glBindBuffer(GL_ARRAY_BUFFER, diffuse_bo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * vertices.size(), diffuse_vec.data());

    // MRA
    glm::vec3 mra = material.getMRA();

    std::vector<glm::vec3> mra_vec;
    for(int i = 0; i < vertices.size(); ++i){
        mra_vec.push_back(mra);
    }

    // USETEX, EMISSIVE, TRANSPARENT
    std::vector<glm::vec3> useT_vec;
    for(int i = 0; i < vertices.size(); ++i){
        useT_vec.push_back(vec3(material.useTexture, material.isEmissive, material.isTransparent));
    }
    glBindBuffer(GL_ARRAY_BUFFER, useTexture_bo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * vertices.size(), useT_vec.data());


    glBindBuffer(GL_ARRAY_BUFFER, mra_bo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * vertices.size(), mra_vec.data());


}

void Mesh::applyTransform(glm::mat4 transform)
{
    for(auto& vert : vertices)
    {
        vert = vec3(transform * glm::vec4(vert,1));
    }

    for(auto& norm : normals)
    {
        norm = vec3(glm::inverse(glm::transpose(transform)) * glm::vec4(norm,1));
        glm::normalize(norm);
    }


    // Bind VAO
    glBindVertexArray(VAO);

    // Bind VBO and copy vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * vertices.size(), vertices.data());

    // Bind normals to layout location 1
    glBindBuffer(GL_ARRAY_BUFFER, NBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * normals.size(), normals.data());
}

void Mesh::createWireframeIndicies()
{
    std::vector<Edge> edges;
    std::unordered_map<Edge, unsigned int[2], EdgeHash, EdgeEqual> edgeFaceAdjacency;

    for (int i = 0; i < indices.size() / 3; ++i) {
        for (int e = 0; e < 3; ++e) {
            int v1 = indices[i*3 + e % 3];
            int v2 = indices[i*3 + (e + 1) % 3];
            Edge edge = std::make_pair(std::min(v1, v2), std::max(v1, v2));

            if (edgeFaceAdjacency.find(edge) == edgeFaceAdjacency.end()) {
                edgeFaceAdjacency[edge][0] = i*3;
                edgeFaceAdjacency[edge][1] = -1;
            } else {
                edgeFaceAdjacency[edge][1] = i*3;
            }
        }
    }


    for (const auto &edge: edgeFaceAdjacency)
    {
        if(edge.second[1] == -1){
            edges.push_back(edge.first);
            continue;
        }

        unsigned int tri1 = edge.second[0];
        unsigned int tri2 = edge.second[1];

        glm::vec3 p0 = vertices[indices[tri1]];
        glm::vec3 p1 = vertices[indices[tri1+1]];
        glm::vec3 p2 = vertices[indices[tri1+2]];

        glm::vec3 p02 = vertices[indices[tri2]];
        glm::vec3 p12 = vertices[indices[tri2+1]];
        glm::vec3 p22 = vertices[indices[tri2+2]];

        glm::vec3 u1 = p1 - p0;
        glm::vec3 v1 = p2 - p0;
        glm::vec3 u2 = p12 - p02;
        glm::vec3 v2 = p22 - p02;


        glm::vec3 n1 = glm::cross(u1,v1);
        glm::vec3 n2 = glm::cross(u2,v2);

        float angle = atan2(glm::length(glm::cross(n1,n2)), glm::dot(n1,n2));

        std::cout << angle << "\n";

        if(angle > 0.77){
            edges.push_back(edge.first);
        }

    }



    for (auto &edge : edges){
        wireframeLineIndicies.push_back(edge.first);
        wireframeLineIndicies.push_back(edge.second);
    }
}
