#include "SceneSerializer.hpp"
#include "Entity.hpp"
#include "Components.hpp"
#include "Asset/AssetManager.hpp"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace YAML
{
	template <>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2 &rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node &node, glm::vec2 &rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template <>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3 &rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node &node, glm::vec3 &rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template <>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4 &rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node &node, glm::vec4 &rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
	YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec2 &v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}
	YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec3 &v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}
	YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec4 &v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}
}
namespace Fermion
{

	SceneSerializer::SceneSerializer(const std::shared_ptr<Scene> &scene) : m_scene(scene)
	{
	}
	static void serializeEntity(YAML::Emitter &out, Entity entity)
	{
		FERMION_ASSERT(entity.hasComponent<IDComponent>(), "Entity must have an IDComponent");
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << entity.getUUID();
		if (entity.hasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Tag" << YAML::Value << entity.getComponent<TagComponent>().tag;
			out << YAML::EndMap;
		}

		if (entity.hasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;
			auto &tc = entity.getComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.getRotationEuler();
			out << YAML::Key << "Scale" << YAML::Value << tc.scale;
			out << YAML::EndMap;
		}
		if (entity.hasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap;
			auto &sprite = entity.getComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << sprite.color;
			if (static_cast<uint64_t>(sprite.textureHandle) != 0)
				out << YAML::Key << "TextureHandle" << YAML::Value << static_cast<uint64_t>(sprite.textureHandle);
			out << YAML::EndMap;
		}
		if (entity.hasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent";
			out << YAML::BeginMap;
			auto &circleComponent = entity.getComponent<CircleRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << circleComponent.color;
			out << YAML::Key << "Thickness" << YAML::Value << circleComponent.thickness;
			out << YAML::Key << "Fade" << YAML::Value << circleComponent.fade;
			out << YAML::EndMap;
		}
		if (entity.hasComponent<TextComponent>())
		{
			out << YAML::Key << "TextComponent";
			out << YAML::BeginMap;
			auto &textComponent = entity.getComponent<TextComponent>();
			out << YAML::Key << "Text" << YAML::Value << textComponent.textString;
			if (static_cast<uint64_t>(textComponent.fontHandle) != 0)
				out << YAML::Key << "FontHandle" << YAML::Value << static_cast<uint64_t>(textComponent.fontHandle);
			out << YAML::Key << "Color" << YAML::Value << textComponent.color;
			out << YAML::Key << "Kerning" << YAML::Value << textComponent.kerning;
			out << YAML::Key << "LineSpacing" << YAML::Value << textComponent.lineSpacing;
			out << YAML::EndMap;
		}
		if (entity.hasComponent<Rigidbody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent";
			out << YAML::BeginMap;

			auto &rb = entity.getComponent<Rigidbody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << static_cast<int>(rb.type);
			out << YAML::Key << "FixedRotation" << YAML::Value << rb.fixedRotation;

			out << YAML::EndMap;
		}
		if (entity.hasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent";
			out << YAML::BeginMap;

			auto &bc = entity.getComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc.offset;
			out << YAML::Key << "Size" << YAML::Value << bc.size;
			out << YAML::Key << "Density" << YAML::Value << bc.density;
			out << YAML::Key << "Friction" << YAML::Value << bc.friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc.restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc.restitutionThreshold;

			out << YAML::EndMap;
		}
		if (entity.hasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent";
			out << YAML::BeginMap;
			auto &cc = entity.getComponent<CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << cc.offset;
			out << YAML::Key << "Radius" << YAML::Value << cc.radius;
			out << YAML::Key << "Density" << YAML::Value << cc.density;
			out << YAML::Key << "Friction" << YAML::Value << cc.friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc.restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc.restitutionThreshold;
			out << YAML::EndMap;
		}
		if(entity.hasComponent<BoxSensor2DComponent>()){
			out << YAML::Key << "BoxSensor2DComponent";
			out << YAML::BeginMap;
			auto &cc = entity.getComponent<BoxSensor2DComponent>();
			out << YAML::Key << "IsTrigger" << YAML::Value << cc.isTrigger;
			out << YAML::Key << "Offset" << YAML::Value << cc.offset;
			out << YAML::Key << "Size" << YAML::Value << cc.size;
			out << YAML::EndMap;
		}
		if (entity.hasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;
			auto &cc = entity.getComponent<CameraComponent>();
			auto &camera = cc.camera;
			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap;
			out << YAML::Key << "ProjectionType" << YAML::Value << static_cast<int>(camera.getProjectionType());
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.getPerspectiveFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.getPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.getPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.getOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.getOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.getOrthographicFarClip();
			out << YAML::EndMap;

			out << YAML::Key << "Primary" << YAML::Value << cc.primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cc.fixedAspectRatio;
			out << YAML::EndMap;
		}
		if (entity.hasComponent<ScriptComponent>())
		{
			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap;
			auto &sc = entity.getComponent<ScriptComponent>();
			out << YAML::Key << "ClassName" << YAML::Value << sc.className;
			out << YAML::EndMap;
		}
		if (entity.hasComponent<ScriptContainerComponent>())
		{
			out << YAML::Key << "ScriptContainerComponent";
			out << YAML::BeginMap;

			auto &scc = entity.getComponent<ScriptContainerComponent>();

			out << YAML::Key << "ScriptClassNames" << YAML::Value << YAML::BeginSeq;
			for (auto &name : scc.scriptClassNames)
				out << name;
			out << YAML::EndSeq;
			out << YAML::EndMap;
		}

		// Close entity map after writing all components
		out << YAML::EndMap;
	}

	void SceneSerializer::serialize(const std::filesystem::path &filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Name";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		auto view = m_scene->m_registry.view<TransformComponent>();
		for (auto entityID : view)
		{
			Entity entity = {entityID, m_scene.get()};
			if (!entity)
				continue;
			serializeEntity(out, entity);
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::serializeRuntime(const std::filesystem::path &filepath)
	{
	}
	bool SceneSerializer::deserialize(const std::filesystem::path &filepath)
	{
		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
		if (!data["Scene"])
			return false;
		std::string sceneName = data["Scene"].as<std::string>();
		Log::Info(std::format("Deserializing scene '{}'", sceneName));
		const auto &entities = data["Entities"];
		if (entities && entities.IsSequence())
		{
			for (auto entity : entities)
			{
				if (!entity.IsMap())
				{
					Log::Warn("Skip non-map entity entry in YAML");
					continue;
				}

				auto idNode = entity["Entity"];
				if (!idNode)
				{
					Log::Warn("Entity missing ID; skipping");
					continue;
				}
				uint64_t uuid = idNode.as<uint64_t>();

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent && tagComponent.IsMap() && tagComponent["Tag"])
				{
					name = tagComponent["Tag"].as<std::string>();
				}
				Log::Info(std::format("Deserialized entity with ID = {0}, name = {1}", uuid, name));
				Entity deserializedEntity = m_scene->createEntityWithUUID(uuid, name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent && transformComponent.IsMap())
				{
					auto &tc = deserializedEntity.getComponent<TransformComponent>();
					if (auto n = transformComponent["Translation"]; n)
						tc.translation = n.as<glm::vec3>();
					if (auto n = transformComponent["Rotation"]; n)
						tc.setRotationEuler(n.as<glm::vec3>());
					if (auto n = transformComponent["Scale"]; n)
						tc.scale = n.as<glm::vec3>();
				}

				auto spriteRendererComponent = entity["SpriteRendererComponent"];
				if (spriteRendererComponent && spriteRendererComponent.IsMap())
				{
					auto &src = deserializedEntity.addComponent<SpriteRendererComponent>();
					if (auto n = spriteRendererComponent["Color"]; n)
						src.color = n.as<glm::vec4>();
					if (auto n = spriteRendererComponent["TextureHandle"]; n)
					{
						uint64_t handleValue = n.as<uint64_t>();
						if (handleValue != 0)
						{
							src.textureHandle = AssetHandle(handleValue);
							src.texture = AssetManager::getAsset<Texture2D>(src.textureHandle);
						}
					}
				}
				auto circleRendererComponent = entity["CircleRendererComponent"];
				if (circleRendererComponent && circleRendererComponent.IsMap())
				{
					auto &cr = deserializedEntity.addComponent<CircleRendererComponent>();
					if (auto n = circleRendererComponent["Color"]; n)
						cr.color = n.as<glm::vec4>();
					if (auto n = circleRendererComponent["Thickness"]; n)
						cr.thickness = n.as<float>();
					if (auto n = circleRendererComponent["Fade"]; n)
						cr.fade = n.as<float>();
				}
				auto textComponent = entity["TextComponent"];
				if (textComponent && textComponent.IsMap())
				{
					auto &tc = deserializedEntity.addComponent<TextComponent>();

					if (auto n = textComponent["Text"]; n)
						tc.textString = n.as<std::string>();

					if (auto n = textComponent["Color"]; n)
						tc.color = n.as<glm::vec4>();
					if (auto n = textComponent["Kerning"]; n)
						tc.kerning = n.as<float>();
					if (auto n = textComponent["LineSpacing"]; n)
						tc.lineSpacing = n.as<float>();
					if (auto n = textComponent["FontHandle"]; n)
					{
						uint64_t handleValue = n.as<uint64_t>();
						if (handleValue != 0)
						{
							tc.fontHandle = AssetHandle(handleValue);
							tc.fontAsset = AssetManager::getAsset<Font>(tc.fontHandle);
						}
					}
				}

				auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
				if (rigidbody2DComponent && rigidbody2DComponent.IsMap())
				{
					auto &rb = deserializedEntity.addComponent<Rigidbody2DComponent>();
					if (auto n = rigidbody2DComponent["BodyType"]; n)
						rb.type = static_cast<Rigidbody2DComponent::BodyType>(n.as<int>());
					if (auto n = rigidbody2DComponent["FixedRotation"]; n)
						rb.fixedRotation = n.as<bool>();
				}

				auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
				if (boxCollider2DComponent && boxCollider2DComponent.IsMap())
				{
					auto &bc = deserializedEntity.addComponent<BoxCollider2DComponent>();
					if (auto n = boxCollider2DComponent["Offset"]; n)
						bc.offset = n.as<glm::vec2>();
					if (auto n = boxCollider2DComponent["Size"]; n)
						bc.size = n.as<glm::vec2>();
					if (auto n = boxCollider2DComponent["Density"]; n)
						bc.density = n.as<float>();
					if (auto n = boxCollider2DComponent["Friction"]; n)
						bc.friction = n.as<float>();
					if (auto n = boxCollider2DComponent["Restitution"]; n)
						bc.restitution = n.as<float>();
					if (auto n = boxCollider2DComponent["RestitutionThreshold"]; n)
						bc.restitutionThreshold = n.as<float>();
				}
				auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
				if (circleCollider2DComponent && circleCollider2DComponent.IsMap())
				{
					auto &cc = deserializedEntity.addComponent<CircleCollider2DComponent>();
					if (auto n = circleCollider2DComponent["Offset"]; n)
						cc.offset = n.as<glm::vec2>();
					if (auto n = circleCollider2DComponent["Radius"]; n)
						cc.radius = n.as<float>();
					if (auto n = circleCollider2DComponent["Density"]; n)
						cc.density = n.as<float>();
					if (auto n = circleCollider2DComponent["Friction"]; n)
						cc.friction = n.as<float>();
					if (auto n = circleCollider2DComponent["Restitution"]; n)
						cc.restitution = n.as<float>();
					if (auto n = circleCollider2DComponent["RestitutionThreshold"]; n)
						cc.restitutionThreshold = n.as<float>();
				}
				auto boxSensor2DComponent = entity["BoxSensor2DComponent"];
				if (boxSensor2DComponent && boxSensor2DComponent.IsMap()) { 
					auto &bs2c = deserializedEntity.addComponent<BoxSensor2DComponent>();
					if (auto n = boxSensor2DComponent["IsTrigger"]; n)
						bs2c.isTrigger = n.as<bool>();
					if (auto n = boxSensor2DComponent["Offset"]; n)
						bs2c.offset = n.as<glm::vec2>();
					if (auto n = boxSensor2DComponent["Size"]; n)
						bs2c.size = n.as<glm::vec2>();
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent && cameraComponent.IsMap())
				{
					auto &cc = deserializedEntity.addComponent<CameraComponent>();
					auto &camera = cc.camera;
					auto camNode = cameraComponent["Camera"];
					if (camNode && camNode.IsMap())
					{
						if (auto n = camNode["ProjectionType"]; n)
							camera.setProjectionType(static_cast<SceneCamera::ProjectionType>(n.as<int>()));
						if (auto n = camNode["PerspectiveFOV"]; n)
							camera.setPerspectiveFOV(n.as<float>());
						if (auto n = camNode["PerspectiveNear"]; n)
							camera.setPerspectiveNearClip(n.as<float>());
						if (auto n = camNode["PerspectiveFar"]; n)
							camera.setPerspectiveFarClip(n.as<float>());
						if (auto n = camNode["OrthographicSize"]; n)
							camera.setOrthographicSize(n.as<float>());
						if (auto n = camNode["OrthographicNear"]; n)
							camera.setOrthographicNearClip(n.as<float>());
						if (auto n = camNode["OrthographicFar"]; n)
							camera.setOrthographicFarClip(n.as<float>());
					}
					if (auto n = cameraComponent["Primary"]; n)
						cc.primary = n.as<bool>();
					if (auto n = cameraComponent["FixedAspectRatio"]; n)
						cc.fixedAspectRatio = n.as<bool>();
				}
				auto scriptComponent = entity["ScriptComponent"];
				if (scriptComponent && scriptComponent.IsMap())
				{
					auto &sc = deserializedEntity.addComponent<ScriptComponent>();
					if (auto n = scriptComponent["ClassName"]; n)
					{
						sc.className = n.as<std::string>();
						Log::Info(std::format("  ScriptComponent: ClassName = {}", sc.className));
					}
				}
				auto scriptContainerNode = entity["ScriptContainerComponent"];
				if (scriptContainerNode && scriptContainerNode.IsMap())
				{
					auto &sc = deserializedEntity.addComponent<ScriptContainerComponent>();

					auto classNamesNode = scriptContainerNode["ScriptClassNames"];
					if (classNamesNode && classNamesNode.IsSequence())
					{
						for (auto classNode : classNamesNode)
							sc.scriptClassNames.push_back(classNode.as<std::string>());
					}
				}
			}
		}
		return true;
	}
	bool SceneSerializer::deserializeRuntime(const std::filesystem::path &filepath)
	{
		return false;
	}
}
