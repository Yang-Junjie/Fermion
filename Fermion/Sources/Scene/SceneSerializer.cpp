#include "SceneSerializer.hpp"
#include "Entity.hpp"
#include "Components.hpp"

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
		out << YAML::BeginMap;
		out << YAML::Key << "Entity" << YAML::Value << "1231313131"; // TODO：Entity ID here
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
			out << YAML::Key << "Color" << YAML::Value << entity.getComponent<SpriteRendererComponent>().color;
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

		// Close entity map after writing all components
		out << YAML::EndMap;
	}

	void SceneSerializer::serialize(const std::string &filepath)
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

	void SceneSerializer::serializeRuntime(const std::string &filepath)
	{
	}
	bool SceneSerializer::deserialize(const std::string &filepath)
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
				Entity deserializedEntity = m_scene->createEntity(name);

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
			}
		}
		return true;
	}
	bool SceneSerializer::deserializeRuntime(const std::string &filepath)
	{
		return false;
	}
}
