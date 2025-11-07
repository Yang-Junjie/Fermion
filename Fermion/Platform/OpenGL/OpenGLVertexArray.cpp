#include "fmpch.hpp"
#include "OpenGLVertexArray.hpp"

#include <glad/glad.h>

namespace Fermion {

	static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:    return GL_FLOAT;
			case ShaderDataType::Float2:   return GL_FLOAT;
			case ShaderDataType::Float3:   return GL_FLOAT;
			case ShaderDataType::Float4:   return GL_FLOAT;
			case ShaderDataType::Mat3:     return GL_FLOAT;
			case ShaderDataType::Mat4:     return GL_FLOAT;
			case ShaderDataType::Int:      return GL_INT;
			case ShaderDataType::Int2:     return GL_INT;
			case ShaderDataType::Int3:     return GL_INT;
			case ShaderDataType::Int4:     return GL_INT;
			case ShaderDataType::Bool:     return GL_BOOL;
		}

		return 0;
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
        FM_PROFILE_FUNCTION();
	
		glCreateVertexArrays(1, &m_rendererID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
        FM_PROFILE_FUNCTION();
		
		glDeleteVertexArrays(1, &m_rendererID);
	}

	void OpenGLVertexArray::bind() const
	{
        FM_PROFILE_FUNCTION();

		glBindVertexArray(m_rendererID);
	}

	void OpenGLVertexArray::unbind() const
	{
        FM_PROFILE_FUNCTION();

		glBindVertexArray(0);
	}

	void OpenGLVertexArray::addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer)
	{
        FM_PROFILE_FUNCTION();

		glBindVertexArray(m_rendererID);
		vertexBuffer->bind();

		const auto& layout = vertexBuffer->getLayout();
		for (const auto& element : layout)
		{
			switch (element.type)
			{
				case ShaderDataType::Float:
				case ShaderDataType::Float2:
				case ShaderDataType::Float3:
				case ShaderDataType::Float4:
				{
					glEnableVertexAttribArray(m_vertexBufferIndex);
					glVertexAttribPointer(m_vertexBufferIndex,
						element.getComponentCount(),
						ShaderDataTypeToOpenGLBaseType(element.type),
						element.normalized ? GL_TRUE : GL_FALSE,
						layout.getStride(),
						(const void*)element.offset);
					m_vertexBufferIndex++;
					break;
				}
				case ShaderDataType::Int:
				case ShaderDataType::Int2:
				case ShaderDataType::Int3:
				case ShaderDataType::Int4:
				case ShaderDataType::Bool:
				{
					glEnableVertexAttribArray(m_vertexBufferIndex);
					glVertexAttribIPointer(m_vertexBufferIndex,
						element.getComponentCount(),
						ShaderDataTypeToOpenGLBaseType(element.type),
						layout.getStride(),
						(const void*)element.offset);
					m_vertexBufferIndex++;
					break;
				}
				case ShaderDataType::Mat3:
				case ShaderDataType::Mat4:
				{
					uint8_t count = element.getComponentCount();
					for (uint8_t i = 0; i < count; i++)
					{
						glEnableVertexAttribArray(m_vertexBufferIndex);
						glVertexAttribPointer(m_vertexBufferIndex,
							count,
							ShaderDataTypeToOpenGLBaseType(element.type),
							element.normalized ? GL_TRUE : GL_FALSE,
							layout.getStride(),
							(const void*)(element.offset + sizeof(float) * count * i));
						glVertexAttribDivisor(m_vertexBufferIndex, 1);
						m_vertexBufferIndex++;
					}
					break;
				}
				default:
					Log::Error("Unknown ShaderDataType!");
			}
		}

		m_vertexBuffers.push_back(vertexBuffer);
	}

	void OpenGLVertexArray::setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
        FM_PROFILE_FUNCTION();

		glBindVertexArray(m_rendererID);
		indexBuffer->bind();

		m_indexBuffer = indexBuffer;
	}

}
