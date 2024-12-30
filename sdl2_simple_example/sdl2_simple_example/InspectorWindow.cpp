#include "InspectorWindow.h"
#include "Variables.h"
#include "Importer.h"
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>
#include <iostream>

extern Importer importer;

struct TriangleFace {
    glm::vec3 normal;
    std::vector<size_t> triangleIndices;
};

bool areNormalsEqual(const glm::vec3& n1, const glm::vec3& n2, float epsilon = 0.0001f) {
    return glm::length(n1 - n2) < epsilon;
}

void InspectorWindow::render(GameObject* selectedObject) {
    ImGui::Begin("Inspector", nullptr);
    if (selectedObject) {
        //GameObject* selectedObject = variables->window->selectedObject;
        if (ImGui::CollapsingHeader("Object Info")) {
            if (ImGui::Checkbox("Is Dynamic", &selectedObject->dynamic)) {
                if (selectedObject->dynamic) {
                    selectedObject->movementState = MovementState::Stopped;
                    selectedObject->speed = 3.0f;
                    selectedObject->movementRange = std::make_pair(-5.0f, 5.0f);
                    selectedObject->movementDirection = 1.0f;
                }
            }

            bool isActive = selectedObject->getActive();
            if (ImGui::Checkbox(" ", &isActive)) {
                selectedObject->setActive(isActive);
            }
            ImGui::SameLine();

            char nameBuffer[256];
            strncpy_s(nameBuffer, selectedObject->name.c_str(), sizeof(nameBuffer) - 1);
            nameBuffer[sizeof(nameBuffer) - 1] = '\0';  // Asegura la terminación nula

            if (ImGui::InputText("Object Name", nameBuffer, sizeof(nameBuffer))) {
                selectedObject->name = std::string(nameBuffer);
            }
        }

        if (ImGui::CollapsingHeader("Transform")) {
            glm::vec3 position = selectedObject->getPosition();
            glm::vec3 rotation = selectedObject->getRotation();
            glm::vec3 scale = selectedObject->getScale();

            if (selectedObject && selectedObject->getActive()) {
                if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f)) {
                    selectedObject->setPosition(position);

                    if (!selectedObject->children.empty()) {
                        selectedObject->updateChildTransforms();
                    }
                }
                if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.1f)) {
                    selectedObject->setRotation(rotation);

                    if (!selectedObject->children.empty()) {
                        selectedObject->updateChildTransforms();
                    }
                }
                if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.1f, 0.1f, 10.0f)) {
                    selectedObject->setScale(scale);

                    if (!selectedObject->children.empty()) {
                        selectedObject->updateChildTransforms();
                    }
                }

                if (ImGui::Button("Reset")) {
                    selectedObject->resetTransform();
                }
            }
            else {
                ImGui::Text("Object is deactivated and cannot be modified");
            }
        }

        MeshData* meshData = selectedObject->getMeshData();
        if (meshData) {
            if (ImGui::CollapsingHeader("Mesh Information")) {
                ImGui::Text("Vertices: %d", meshData->vertices.size() / 3);
                ImGui::Text("Indices: %d", meshData->indices.size() / 3);

                if (ImGui::CollapsingHeader("Show Normals")) {
                    if (meshData->vertices.size() / 3 > 0) {
                        std::unordered_map<std::string, TriangleFace> faces;
                        ImGui::Text("---Triangle Normals---");
                        for (size_t i = 0; i < meshData->indices.size(); i += 3) {
                            glm::vec3 vertex1 = glm::vec3(meshData->vertices[meshData->indices[i] * 3], meshData->vertices[meshData->indices[i] * 3 + 1], meshData->vertices[meshData->indices[i] * 3 + 2]);
                            glm::vec3 vertex2 = glm::vec3(meshData->vertices[meshData->indices[i + 1] * 3], meshData->vertices[meshData->indices[i + 1] * 3 + 1], meshData->vertices[meshData->indices[i + 1] * 3 + 2]);
                            glm::vec3 vertex3 = glm::vec3(meshData->vertices[meshData->indices[i + 2] * 3], meshData->vertices[meshData->indices[i + 2] * 3 + 1], meshData->vertices[meshData->indices[i + 2] * 3 + 2]);

                            glm::vec3 edge1 = vertex2 - vertex1;
                            glm::vec3 edge2 = vertex3 - vertex1;
                            glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

                            ImGui::Text("Triangle %d Normal: %.3f, %.3f, %.3f", i / 3, faceNormal.x, faceNormal.y, faceNormal.z);

                            std::string normalKey = std::to_string(faceNormal.x) + "," + std::to_string(faceNormal.y) + "," + std::to_string(faceNormal.z);

                            bool found = false;
                            for (auto& pair : faces) {
                                if (areNormalsEqual(pair.second.normal, faceNormal)) {
                                    pair.second.triangleIndices.push_back(i / 3);
                                    found = true;
                                    break;
                                }
                            }

                            if (!found) {
                                faces[normalKey] = TriangleFace{ faceNormal, {i / 3} };
                            }
                        }
                        ImGui::Separator();
                        ImGui::Text("---Face Normals---");
                        for (const auto& faceEntry : faces) {
                            const TriangleFace& face = faceEntry.second;
                            ImGui::Text("Face Normal: %.3f, %.3f, %.3f", face.normal.x, face.normal.y, face.normal.z);
                            for (int triangleIndex : face.triangleIndices) {
                                ImGui::Text("  Triangle Index: %d", triangleIndex);
                            }
                        }
                    }
                }
            }
        }

        if (ImGui::CollapsingHeader("Texture Information")) {
            ImGui::TextWrapped("Object Path: %s", variables->textureFilePath.c_str());

            importer.getTextureDimensions(selectedObject->textureID, variables->texturewidth, variables->textureheight);
            ImGui::Text("Texture Dimensions: %d x %d", variables->texturewidth, variables->textureheight);

            if (selectedObject->textureID != 0) {
                ImGui::Separator();
                ImGui::Text("Object Texture:");
                ImVec2 textureSize(variables->texturewidth, variables->textureheight);
                ImGui::Image((void*)(intptr_t)selectedObject->textureID, ImVec2(150, 150), ImVec2(0, 1), ImVec2(1, 0));
            }
            else {
                ImGui::Text("No texture assigned");
            }

            if (ImGui::Button("Checker Texture")) {
                GLuint newTextureID = importer.loadTexture(variables->checkerTexture);
                variables->window->selectedObject->textureID = newTextureID;

                variables->textureFilePath = variables->checkerTexture;
            }
        }
    }
    ImGui::End();
}
