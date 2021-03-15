#include <glr/obj.h>

#ifdef GLRENDER_STATIC
#include <glad/glad.h>
#endif

namespace glr
{

	GLRENDER_INLINE OBJ::OBJ(std::string obj_path, std::string base_dir, std::string obj_name, bool calc_normals, bool flip_normals)
	{
		loadFromObj(obj_path, base_dir, obj_name, calc_normals, flip_normals);
	}

	GLRENDER_INLINE OBJ::OBJ(const OBJ &src)
	{
		*this = src;
	}

	GLRENDER_INLINE void OBJ::operator=(const OBJ &src)
	{
		loadFromObj(src.obj_path_, src.base_dir_, src.name_, src.is_calc_normals_, src.is_flip_normals_);

		this->shader_list_ = src.shader_list_;
		this->texture_list_ = src.texture_list_;
		this->textures_assigned_ = src.textures_assigned_;
		this->use_vert_colors_ = src.use_vert_colors_;

		this->attrib_ = src.attrib_;
		this->shapes_ = src.shapes_;
		this->materials_ = src.materials_;

		initGLBuffers(src.is_calc_normals_, src.is_calc_normals_);
	}

	GLRENDER_INLINE void OBJ::loadFromObj(std::string obj_path, std::string base_dir, std::string obj_name, bool calc_normals, bool flip_normals)
	{

		this->name_ = obj_name;

		this->obj_path_ = obj_path;
		if (base_dir[base_dir.length() - 1] != '/')
			base_dir += "/";
		this->base_dir_ = base_dir;

		std::string warn;
		std::string err;

		bool ret = tinyobj::LoadObj(&this->attrib_, &this->shapes_, &this->materials_, &warn, &err, obj_path.c_str(), base_dir.c_str());

		if (this->materials_.size() == 0)
		{
			tinyobj::material_t default_mat;
			default_mat.name = "default";
			default_mat.ambient[0] = 1;
			default_mat.ambient[1] = 1;
			default_mat.ambient[2] = 1;
			default_mat.diffuse[0] = 1;
			default_mat.diffuse[1] = 1;
			default_mat.diffuse[2] = 1;
			default_mat.specular[0] = 1;
			default_mat.specular[1] = 1;
			default_mat.specular[2] = 1;
			default_mat.emission[0] = 0;
			default_mat.emission[1] = 0;
			default_mat.emission[2] = 0;
			default_mat.shininess = 1000;

			materials_.push_back(default_mat);

			for (int s = 0; s < shapes_.size(); s++)
			{
				shapes_[s].mesh.material_ids.clear();
				shapes_[s].mesh.material_ids.push_back(0);
			}
		}

		if (!warn.empty())
		{
			std::cout << warn << std::endl;
		}

		if (!err.empty())
		{
			std::cerr << err << std::endl;
		}

		if (!ret)
		{
			exit(1);
		}

		shader_list_.clear();
		texture_list_.clear();
		textures_assigned_.clear();

		for (int s = 0; s < shapes_.size(); s++)
		{
			shader_list_.push_back(NULL);
			texture_list_.push_back(NULL);
			textures_assigned_.push_back(false);
		}

		this->no_uv_map_.resize(this->shapes_.size());

		initGLBuffers(calc_normals, flip_normals);

		calcCenters();

		aabb_tree_.assignObj(this);
		if (aabb_tree_enabled_)
			aabb_tree_.calcTree();
		obb_tree_.assignObj(this);
		if (obb_tree_enabled_)
			obb_tree_.calcTree();

		this->use_vert_colors_.clear();
		for (int s = 0; s < shapes_.size(); s++)
		{
			this->use_vert_colors_.push_back(false);
		}
	}

	GLRENDER_INLINE void OBJ::setVertColor(float color[3])
	{
		for (int s = 0; s < shapes_.size(); s++)
			setVertColorForShape(shapes_[s].name, color);
	}

	GLRENDER_INLINE void OBJ::setVertColorForShape(std::string shapeName, float color[3])
	{
		tinyobj::attrib_t &attrib = this->attrib_;

		int s;
		for (s = 0; s < this->shapes_.size(); s++)
			if (shapeName == this->shapes_[s].name)
				break;

		// loop over faces
		size_t index_offset = 0;
		for (size_t f = 0; f < this->shapes_[s].mesh.num_face_vertices.size(); f++)
		{
			if (this->shapes_[s].mesh.num_face_vertices[f] != 3)
			{
				index_offset += this->shapes_[s].mesh.num_face_vertices[f];
				continue;
			}

			// Loop over vertices in the face.
			for (size_t v = 0; v < 3; v++)
			{
				tinyobj::index_t idx = this->shapes_[s].mesh.indices[index_offset + v];
				attrib.colors[3 * idx.vertex_index + 0] = color[0];
				attrib.colors[3 * idx.vertex_index + 1] = color[1];
				attrib.colors[3 * idx.vertex_index + 2] = color[2];
			}

			index_offset += 3;
		}

		initGLBuffers(is_calc_normals_, is_flip_normals_);
	}

	GLRENDER_INLINE void OBJ::useVertColor(bool use)
	{
		for (int s = 0; s < shapes_.size(); s++)
			use_vert_colors_[s] = use;
	}

	GLRENDER_INLINE void OBJ::useVertColorForShape(std::string shape_name, bool use)
	{
		for (int s = 0; s < shapes_.size(); s++)
			if (shapes_[s].name == shape_name)
				use_vert_colors_[s] = use;
	}

	GLRENDER_INLINE void OBJ::setShader(shader *shader_ptr)
	{
		for (int s = 0; s < shapes_.size(); s++)
			shader_list_[s] = shader_ptr;
	}

	GLRENDER_INLINE void OBJ::setShaderForShape(std::string shape_name, shader *shader_ptr)
	{
		for (int s = 0; s < shapes_.size(); s++)
			if (shapes_[s].name == shape_name)
				shader_list_[s] = shader_ptr;
	}

	GLRENDER_INLINE void OBJ::setTexture(texture *texture_ptr)
	{
		for (int s = 0; s < shapes_.size(); s++)
		{
			texture_list_[s] = texture_ptr;
			if (texture_ptr != NULL)
				textures_assigned_[s] = true;
			else
				textures_assigned_[s] = false;
		}
	}

	GLRENDER_INLINE void OBJ::setTextureForShape(std::string shape_name, texture *texture_ptr)
	{
		for (int s = 0; s < shapes_.size(); s++)
			if (shapes_[s].name == shape_name)
			{
				texture_list_[s] = texture_ptr;
				if (texture_ptr != NULL)
					textures_assigned_[s] = true;
				else
					textures_assigned_[s] = false;
			}
	}

	GLRENDER_INLINE glm::mat4 OBJ::modelMatrix()
	{
		return model_matrix_;
	}

	GLRENDER_INLINE void OBJ::modelMatrix(glm::mat4 mat)
	{
		model_matrix_ = mat;
	}

	GLRENDER_INLINE void OBJ::enableAABB(bool use)
	{
		if (use)
		{
			enableOBB(false);
			displayOBB(false);
			aabb_tree_.calcTree();
		}
		else
			aabb_tree_.clearTree();
		
		aabb_tree_enabled_ = use;
	}

	GLRENDER_INLINE void OBJ::displayAABB(bool use)
	{
		if (use && aabb_tree_enabled_)
		{
			display_aabb_tree_ = use;
			aabb_tree_.initGLBuffers();
		}
		else
			display_aabb_tree_ = false;
	}

	GLRENDER_INLINE void OBJ::enableOBB(bool use)
	{
		if (use)
		{
			enableAABB(false);
			displayAABB(false);
			obb_tree_.calcTree();
		}
		else
			obb_tree_.clearTree();
		
		obb_tree_enabled_ = use;
	}

	GLRENDER_INLINE void OBJ::displayOBB(bool use)
	{
		if (use && obb_tree_enabled_)
		{
			display_obb_tree_ = use;
			obb_tree_.initGLBuffers();
		}
		else
			display_obb_tree_ = false;
	}

	GLRENDER_INLINE bool OBJ::isIntersect(OBJ* other_obj)
	{
		bool is_intersect = false;
		if (aabb_tree_enabled_)
		{
			is_intersect = this->aabb_tree_.intersectTest( &(other_obj->aabb_tree_) );

			this->displayAABB(this->display_aabb_tree_);
			other_obj->displayAABB(other_obj->display_aabb_tree_);
		}
		else if (obb_tree_enabled_)
		{
			is_intersect = this->obb_tree_.intersectTest( &(other_obj->obb_tree_) );

			this->displayOBB(this->display_obb_tree_);
			other_obj->displayOBB(other_obj->display_obb_tree_);
		}

		return is_intersect;
	}

	GLRENDER_INLINE void OBJ::draw()
	{
		int shape_num = shapes_.size();
		for (int s = 0; s < shape_num; s++)
		{
			// skip meshes with no faces (i.e. a point)
			if (shapes_[s].mesh.num_face_vertices.size() == 0)
				continue;

			//enable shader
			shader_list_[s]->use();

			// enable texture if there is one and if a uvmap exists
			if (texture_list_[s] != NULL)
				texture_list_[s]->bind();
			int texture_assigned = (!textures_assigned_[s]) ? 0 : 1;
			// if the user tried to assign a texture to a shape
			// with no UVMap, "empty" texture is assigned
			// if they texture map in a shader, they will
			// get white
			if (this->no_uv_map_[s])
				texture_assigned = 0;
			glUniform1i(glGetUniformLocation(shader_list_[s]->ID_, "textureAssigned"), texture_assigned);

			// enable vertex shading if specified
			int useVertColor = (this->use_vert_colors_[s]) ? 1 : 0;
			glUniform1i(glGetUniformLocation(shader_list_[s]->ID_, "useVertColor"), useVertColor);

			// set up uniforms
			tinyobj::material_t mat = materials_[shapes_[s].mesh.material_ids[0]];
			setUniforms(s, mat, shader_list_[s]);

			// draw
			glBindVertexArray(vao_list_[s]);
			glDrawArrays(GL_TRIANGLES, 0, 3 * shapes_[s].mesh.num_face_vertices.size());
			glBindVertexArray(0);
		}

		if (display_aabb_tree_)
		{
			AABBTree::aabb_shader_.use();
			aabb_tree_.draw();
		}
		else if (display_obb_tree_)
		{
			OBBTree::obb_shader_.use();
			obb_tree_.draw();
		}
	}

	GLRENDER_INLINE void OBJ::glRelease()
	{
		if (!is_loaded_into_gl_)
			return;

		glDeleteBuffers(this->vbo_list_.size(), this->vbo_list_.data());
		glDeleteVertexArrays(this->vao_list_.size(), this->vao_list_.data());

		this->vao_list_.clear();
		this->vbo_list_.clear();

		is_loaded_into_gl_ = false;
	}

	GLRENDER_INLINE OBJ::~OBJ()
	{
		glRelease();
	}

	GLRENDER_INLINE void OBJ::setUniforms(unsigned int shape_idx, tinyobj::material_t &mat, shader *shader_ptr)
	{
		// model matrix
		int u_location = glGetUniformLocation(shader_ptr->ID_, "m");
		glUniformMatrix4fv(u_location, 1, GL_FALSE, glm::value_ptr(this->model_matrix_));

		// material info
		shader_ptr->setVec3("Ka", mat.ambient);
		shader_ptr->setVec3("Kd", mat.diffuse);
		shader_ptr->setVec3("Ks", mat.specular);
		shader_ptr->setVec3("Ke", mat.emission);
		shader_ptr->setFloat("Ns", mat.shininess);

	

		// model matrix
		if (display_aabb_tree_)
		{
			shader* aabb_shader_ptr = &AABBTree::aabb_shader_;
			aabb_shader_ptr->use();
			int uLocation = glGetUniformLocation(aabb_shader_ptr->ID_, "m");
			glUniformMatrix4fv(uLocation, 1, GL_FALSE, glm::value_ptr(model_matrix_));
		}
		else if (display_obb_tree_)
		{
			shader* obb_shader_ptr = &OBBTree::obb_shader_;
			obb_shader_ptr->use();
			int uLocation = glGetUniformLocation(obb_shader_ptr->ID_, "m");
			glUniformMatrix4fv(uLocation, 1, GL_FALSE, glm::value_ptr(model_matrix_));
		}

		shader_ptr->use();
	}

	GLRENDER_INLINE void OBJ::initGLBuffers(bool calc_normals, bool flip_normals)
	{

		this->is_calc_normals_ = calc_normals;
		this->is_flip_normals_ = flip_normals;

		if (this->is_loaded_into_gl_)
			this->glRelease();

		std::string baseDir = this->base_dir_;

		int shape_num = shapes_.size();
		std::vector<float> vertex_data;
		vao_list_.clear();
		vao_list_.resize(shape_num);
		vbo_list_.clear();
		unsigned int VBO;

		// generate vertex arrays
		glGenVertexArrays(shape_num, vao_list_.data());

		// loop over shapes
		size_t v_count = 0;
		for (int s = 0; s < shape_num; s++)
		{
			vertex_data.clear();
			v_count = 0;

			// loop over faces
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes_[s].mesh.num_face_vertices.size(); f++)
			{
				if (shapes_[s].mesh.num_face_vertices[f] != 3)
				{
					index_offset += shapes_[s].mesh.num_face_vertices[f];
					continue;
				}

				// Loop over vertices in the face.
				for (size_t v = 0; v < 3; v++)
				{
					// access to vertex
					tinyobj::index_t idx = shapes_[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib_.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib_.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib_.vertices[3 * idx.vertex_index + 2];

					tinyobj::real_t nx;
					tinyobj::real_t ny;
					tinyobj::real_t nz;
					if (idx.normal_index != -1)
					{
						nx = ((int)flip_normals * -2 + 1) * attrib_.normals[3 * idx.normal_index + 0];
						ny = ((int)flip_normals * -2 + 1) * attrib_.normals[3 * idx.normal_index + 1];
						nz = ((int)flip_normals * -2 + 1) * attrib_.normals[3 * idx.normal_index + 2];
					}
					else
					{
						is_calc_normals_ = true;
						calc_normals = true;
					}

					tinyobj::real_t tx;
					tinyobj::real_t ty;
					if (idx.texcoord_index != -1)
					{
						tx = attrib_.texcoords[2 * idx.texcoord_index + 0];
						ty = attrib_.texcoords[2 * idx.texcoord_index + 1];
					}
					else
					{
						tx = 0;
						ty = 0;
						this->no_uv_map_[s] = true;
					}

					// Optional: vertex colors
					tinyobj::real_t r = attrib_.colors[3 * idx.vertex_index + 0];
					tinyobj::real_t g = attrib_.colors[3 * idx.vertex_index + 1];
					tinyobj::real_t b = attrib_.colors[3 * idx.vertex_index + 2];

					vertex_data.push_back(vx);
					vertex_data.push_back(vy);
					vertex_data.push_back(vz);
					vertex_data.push_back(nx);
					vertex_data.push_back(ny);
					vertex_data.push_back(nz);
					vertex_data.push_back(r);
					vertex_data.push_back(g);
					vertex_data.push_back(b);
					vertex_data.push_back(tx);
					vertex_data.push_back(ty);

					v_count++;
				}
				if (calc_normals)
				{
					glm::vec3 v3(vertex_data[11 * (v_count - 3) + 0], vertex_data[11 * (v_count - 3) + 1], vertex_data[11 * (v_count - 3) + 2]);
					glm::vec3 v2(vertex_data[11 * (v_count - 2) + 0], vertex_data[11 * (v_count - 2) + 1], vertex_data[11 * (v_count - 2) + 2]);
					glm::vec3 v1(vertex_data[11 * (v_count - 1) + 0], vertex_data[11 * (v_count - 1) + 1], vertex_data[11 * (v_count - 1) + 2]);

					glm::vec3 v12 = v2 - v1;
					glm::vec3 v13 = v3 - v2;
					glm::vec3 n = glm::normalize(glm::cross(v13, v12));
					n *= ((int)flip_normals * -2 + 1);
					(vertex_data[11 * (v_count - 3) + 3 + 0]) = n.x;
					(vertex_data[11 * (v_count - 3) + 3 + 1]) = n.y;
					(vertex_data[11 * (v_count - 3) + 3 + 2]) = n.z;
					(vertex_data[11 * (v_count - 2) + 3 + 0]) = n.x;
					(vertex_data[11 * (v_count - 2) + 3 + 1]) = n.y;
					(vertex_data[11 * (v_count - 2) + 3 + 2]) = n.z;
					(vertex_data[11 * (v_count - 1) + 3 + 0]) = n.x;
					(vertex_data[11 * (v_count - 1) + 3 + 1]) = n.y;
					(vertex_data[11 * (v_count - 1) + 3 + 2]) = n.z;
				}
				index_offset += 3;
			}

			// this->shapeCenters.push_back(getShapeCenter(obj, s));

			glBindVertexArray(vao_list_[s]);

			glGenBuffers(1, &VBO);
			this->vbo_list_.push_back(VBO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);

			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_data.size(), vertex_data.data(), GL_DYNAMIC_DRAW);

			// position attribute
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)0);
			glEnableVertexAttribArray(0);
			// normal attribute
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			// color attribute
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)(6 * sizeof(float)));
			glEnableVertexAttribArray(2);
			// uv coord attribute
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void *)(9 * sizeof(float)));
			glEnableVertexAttribArray(3);

			glBindVertexArray(0);
		}

		this->is_loaded_into_gl_ = true;
	}

	GLRENDER_INLINE void OBJ::calcCenters()
	{
		float x_bounds[2];
		float y_bounds[2];
		float z_bounds[2];

		x_bounds[0] = attrib_.vertices[0];
		x_bounds[1] = attrib_.vertices[0];
		y_bounds[0] = attrib_.vertices[1];
		y_bounds[1] = attrib_.vertices[1];
		z_bounds[0] = attrib_.vertices[2];
		z_bounds[1] = attrib_.vertices[2];

		shape_centers_.clear();
		shape_radii_.clear();

		for (int s = 0; s < shapes_.size(); s++)
		{
			float x_bounds_shape[2];
			float y_bounds_shape[2];
			float z_bounds_shape[2];

			tinyobj::index_t idx = shapes_[s].mesh.indices[0];
			tinyobj::real_t vx = attrib_.vertices[3 * idx.vertex_index + 0];
			tinyobj::real_t vy = attrib_.vertices[3 * idx.vertex_index + 1];
			tinyobj::real_t vz = attrib_.vertices[3 * idx.vertex_index + 2];

			x_bounds_shape[0] = vx;
			x_bounds_shape[1] = vx;
			y_bounds_shape[0] = vy;
			y_bounds_shape[1] = vy;
			z_bounds_shape[0] = vz;
			z_bounds_shape[1] = vz;

			// loop over faces
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes_[s].mesh.num_face_vertices.size(); f++)
			{
				if (shapes_[s].mesh.num_face_vertices[f] != 3)
				{
					index_offset += shapes_[s].mesh.num_face_vertices[f];
					continue;
				}
				// Loop over vertices in the face.
				for (size_t v = 0; v < 3; v++)
				{
					// access to vertex
					idx = shapes_[s].mesh.indices[index_offset + v];
					vx = attrib_.vertices[3 * idx.vertex_index + 0];
					vy = attrib_.vertices[3 * idx.vertex_index + 1];
					vz = attrib_.vertices[3 * idx.vertex_index + 2];

					if (vx < x_bounds_shape[0])
						x_bounds_shape[0] = vx;
					if (vx > x_bounds_shape[1])
						x_bounds_shape[1] = vx;
					if (vy < y_bounds_shape[0])
						y_bounds_shape[0] = vy;
					if (vy > y_bounds_shape[1])
						y_bounds_shape[1] = vy;
					if (vz < z_bounds_shape[0])
						z_bounds_shape[0] = vz;
					if (vz > z_bounds_shape[1])
						z_bounds_shape[1] = vz;

					if (vx < x_bounds[0])
						x_bounds[0] = vx;
					if (vx > x_bounds[1])
						x_bounds[1] = vx;
					if (vy < y_bounds[0])
						y_bounds[0] = vy;
					if (vy > y_bounds[1])
						y_bounds[1] = vy;
					if (vz < z_bounds[0])
						z_bounds[0] = vz;
					if (vz > z_bounds[1])
						z_bounds[1] = vz;
				}

				index_offset += 3;
			}

			glm::vec3 shape_center;
			shape_center.x = (x_bounds_shape[0] + x_bounds_shape[1]) / 2;
			shape_center.y = (y_bounds_shape[0] + y_bounds_shape[1]) / 2;
			shape_center.z = (z_bounds_shape[0] + z_bounds_shape[1]) / 2;

			shape_centers_.push_back(shape_center);

			index_offset = 0;
			float radius = 0;
			for (size_t f = 0; f < shapes_[s].mesh.num_face_vertices.size(); f++)
			{
				if (shapes_[s].mesh.num_face_vertices[f] != 3)
				{
					index_offset += shapes_[s].mesh.num_face_vertices[f];
					continue;
				}
				// Loop over vertices in the face.
				for (size_t v = 0; v < 3; v++)
				{
					// access to vertex
					idx = shapes_[s].mesh.indices[index_offset + v];
					vx = attrib_.vertices[3 * idx.vertex_index + 0];
					vy = attrib_.vertices[3 * idx.vertex_index + 1];
					vz = attrib_.vertices[3 * idx.vertex_index + 2];
				}

				index_offset += 3;

				glm::vec3 v = glm::vec3(vx, vy, vz);

				float tmp = glm::length(v - shape_center);
				if (tmp > radius)
					radius = tmp;
			}

			shape_radii_.push_back(radius);
		}

		center_.x = (x_bounds[0] + x_bounds[1]) / 2;
		center_.y = (y_bounds[0] + y_bounds[1]) / 2;
		center_.z = (z_bounds[0] + z_bounds[1]) / 2;

		radius_ = 0;

		for (int s = 0; s < shapes_.size(); s++)
		{

			tinyobj::index_t idx;
			tinyobj::real_t vx;
			tinyobj::real_t vy;
			tinyobj::real_t vz;

			// loop over faces
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes_[s].mesh.num_face_vertices.size(); f++)
			{
				if (shapes_[s].mesh.num_face_vertices[f] != 3)
				{
					index_offset += shapes_[s].mesh.num_face_vertices[f];
					continue;
				}
				// Loop over vertices in the face.
				for (size_t v = 0; v < 3; v++)
				{
					// access to vertex
					idx = shapes_[s].mesh.indices[index_offset + v];
					vx = attrib_.vertices[3 * idx.vertex_index + 0];
					vy = attrib_.vertices[3 * idx.vertex_index + 1];
					vz = attrib_.vertices[3 * idx.vertex_index + 2];
				}

				index_offset += 3;

				glm::vec3 v = glm::vec3(vx, vy, vz);

				float tmp = glm::length(v - center_);
				if (tmp > radius_)
					radius_ = tmp;
			}
		}
	}

} // namespace glr