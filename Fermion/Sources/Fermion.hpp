#pragma once

// Core
#include "Core/Application.hpp"
#include "Core/Input.hpp"
#include "Core/KeyCodes.hpp"
#include "Core/Log.hpp"
#include "Core/Layer.hpp"
#include "Core/LayerStack.hpp"
#include "Core/Timestep.hpp"
#include "Core/Window.hpp"

// Events
#include "Events/Event.hpp"
#include "Events/ApplicationEvent.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseEvent.hpp"

// ImGui
#include "ImGui/ImGuiLayer.hpp"

// Scene
#include "Scene/Scene.hpp"
#include "Scene/Components.hpp"
#include "Scene/Entity.hpp"
#include "Scene/ScriptableEntity.hpp"

// Renderer
#include "Renderer/Buffer.hpp"
#include "Renderer/GraphicsContext.hpp"
#include "Renderer/Camera/OrthographicCamera.hpp"
#include "Renderer/Camera/OrthographicCameraController.hpp"
#include "Renderer/Renderers/Renderer.hpp"
#include "Renderer/RendererConfig.hpp"
#include "Renderer/RendererAPI.hpp"
#include "Renderer/Renderers/Renderer2D.hpp"
#include "Renderer/Renderers/Renderer2DCompat.hpp"
#include "Renderer/Renderers/SceneRenderer.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/Texture/Texture.hpp"
#include "Renderer/Texture/SubTexture2D.hpp"
#include "Renderer/Framebuffer.hpp"

// Project
#include "Project/Project.hpp"

// Asset system
#include "Asset/AssetManager.hpp"
#include "Asset/AssetManager/RuntimeAssetManager.hpp"
#include "Asset/AssetManager/EditorAssetManager.hpp"
