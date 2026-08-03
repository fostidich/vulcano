// clang-format off
// NOLINTBEGIN(*)
#pragma once

#include "Colliders.hpp"
#include "Starter.hpp"
#include <glm/glm.hpp>
#include <json.hpp>
#include <string>
#include <vector>

struct TechniqueInstances;

struct Instance {
	std::string *id;
	int Mid;
	int NTx;
	int Iid;
	int *Tid;
	DescriptorSet ***DS;
	std::vector<DescriptorSetLayout *> **D;
	int *NDs;

	glm::mat4 Wm;
	TechniqueInstances *TIp;

	Collider *C = nullptr; // Pointer to Instance's Collider
} ;

struct TextureDefs {
	bool fromInstance;
	int pos;
	VkDescriptorImageInfo info;
} ;

struct PipelineAndTexturesDefs {
	Pipeline *P;
	std::vector<std::vector<TextureDefs>> texDefs;
} ;

struct TechniqueRef {
	std::string *id;
	std::vector<PipelineAndTexturesDefs>PT;
	int Ntextures;
	VertexDescriptor *VD;

	void init(const char *_id, std::vector<PipelineAndTexturesDefs> _PT, int _Ntextures, VertexDescriptor * _VD);
} ;

struct VertexDescriptorRef {
	std::string *id;
	VertexDescriptor *VD;

	void init(const char *_id, VertexDescriptor * _VD);
} ;

struct TechniqueInstances {
	Instance *I;
	int InstanceCount;

	TechniqueRef *T;
} ;

// Struct to store the Collider's definition read from "models" section
struct ColliderDef {
	bool hasCollider = false;
	std::string type;	// AABB, OOBB, Sphere, Point, BVH
	std::vector<float> params;	// Collider's parameters
	bool visible = false;		// Flag to enable Collider's visualization
	std::vector<ColliderDef> children; // Recursive definition for BVH's children
};


class Scene {
	public:

	BaseProject *BP;

	// Models, textures and Descriptors (values assigned to the uniforms)
	// Please note that Model objects depends on the corresponding vertex structure
	// Asset files
	int AssetFileCount = 0;
	AssetFile **As;
	std::unordered_map<std::string, int> AsIds;

	// Models
	int ModelCount = 0;
	Model **M;
	std::unordered_map<std::string, int> MeshIds;

	// Textures
	int TextureCount = 0;
	Texture **T;
	std::unordered_map<std::string, int> TextureIds;

	// Descriptor sets and instances
	int InstanceCount = 0;

	Instance **I;
	VertexDescriptorRef *VRef;
	std::unordered_map<std::string, int> InstanceIds;

	// Pipelines, DSL and Vertex Formats
	std::unordered_map<std::string, TechniqueRef *> TechniqueIds;
	int TechniqueInstanceCount = 0;
	TechniqueInstances *TI;
	std::unordered_map<std::string, VertexDescriptor *> VDIds;
	int Npasses;

	// Colliders' definitions (mapped index by index with the Models)
	std::vector<ColliderDef> ModelColliderDefs;

	// Global List for the memory cleanup of the instantiated colliders
	std::vector<Collider *> GlobalColliders;

	// Colliders visualization
	ColliderShow ColShow;

	int init(BaseProject *_BP,  int _Npasses, std::vector<VertexDescriptorRef>  &VDRs, std::vector<TechniqueRef> &PRs, std::string file);

	void pipelinesAndDescriptorSetsInit();
	void pipelinesAndDescriptorSetsCleanup();
	void localCleanup();
    void populateCommandBuffer(VkCommandBuffer commandBuffer, int passId, int currentImage);
	void updateColliderVisualizer(uint32_t currentImage, glm::mat4 ViewPrj);
	void refreshColliderVisualizer();
	void ParseColliderRecursive(nlohmann::json &node, ColliderDef &def, Model **M, int modelIndex);
	Collider* CreateColliderRecursive(ColliderDef &def, std::vector<Collider *> &GlobalList, ColliderShow &Show, bool parentVisible);
	void setColliderStroke(Collider* c, glm::vec4 color);
};

#ifdef SCENE_IMPLEMENTATION

void TechniqueRef::init(const char *_id, std::vector<PipelineAndTexturesDefs> _PT, int _Ntextures, VertexDescriptor * _VD) {
	id = new std::string(_id);
	PT = _PT;
	Ntextures = _Ntextures;
	VD = _VD;
}

void VertexDescriptorRef::init(const char *_id, VertexDescriptor * _VD) {
	id = new std::string(_id);
	VD = _VD;
}

int Scene::init(BaseProject *_BP,  int _Npasses, std::vector<VertexDescriptorRef>  &VDRs,
		  std::vector<TechniqueRef> &PRs, std::string file) {
	BP = _BP;
	Npasses = _Npasses;

	for(int i = 0; i < VDRs.size(); i++) {
		VDIds[*VDRs[i].id] = VDRs[i].VD;
	}
	for(int i = 0; i < PRs.size(); i++) {
		if(PRs[i].PT.size() != Npasses) {
			logs::error("Scene Error: the number of pipelines for technique ", *PRs[i].id, " does not correspond to the number of passes : ", PRs[i].PT.size(), " != ", Npasses);
			exit(0);
		}
		TechniqueIds[*PRs[i].id] = &PRs[i];
	}

	// Models, textures and Descriptors (values assigned to the uniforms)
	nlohmann::json js;
	std::ifstream ifs(file);
	if (!ifs.is_open()) {
	  logs::error("Error! Scene file >", file, "< not found!");
	  exit(-1);
	}
//		try {
		logs::debug("Parsing JSON");
		ifs >> js;
		ifs.close();
		logs::debug("Scene contains ", js.size(), " definitions sections");

		// ASSET FILES
		nlohmann::json afs = js["assetfiles"];
		AssetFileCount = afs.size();
		logs::debug("Asset Files count: ", AssetFileCount);

		As = (AssetFile **)calloc(AssetFileCount, sizeof(AssetFile *));
		for(int k = 0; k < AssetFileCount; k++) {
			AsIds[afs[k]["id"]] = k;
			std::string MT = afs[k]["format"].template get<std::string>();

			As[k] = new AssetFile();
			As[k]->init(afs[k]["file"], (MT[0] == 'O') ? OBJ : ((MT[0] == 'G') ? GLTF : MGCG));
			if (MT[0] == 'G') {
				// Solo se è un GLTF
				tinygltf::TinyGLTF loader;
				tinygltf::Model model;
				std::string warn, err;
				std::string path = afs[k]["file"];

				bool ok = loader.LoadASCIIFromFile(&model, &warn, &err, path);
				if (!ok) {
					logs::error("Failed to load GLTF file for debug: ", path, " - Error: ", err);
				} else {
					logs::debug("=== DEBUG INFO FROM: ", path, " ===");
					for (size_t m = 0; m < model.meshes.size(); ++m) {
						const auto& mesh = model.meshes[m];
						logs::debug("Mesh ", m, ": ", mesh.name);
						for (size_t p = 0; p < mesh.primitives.size(); ++p) {
							const auto& prim = mesh.primitives[p];
							logs::debug("  Primitive ", p, ":");
							for (const auto& attr : prim.attributes) {
								logs::debug("    Attribute: ", attr.first);
							}
						}
					}
					logs::debug("Skins: ", model.skins.size());
					logs::debug("Animations: ", model.animations.size());
					logs::debug("===============================");
				}
			}

		}

		// Initialize Colliders visualizer (ex: max 20 colliders visualized)
		ColShow.init(BP, 20);

		// MODELS
		nlohmann::json ms = js["models"];
		ModelCount = ms.size();
		logs::debug("Models count: ", ModelCount);

		M = (Model **)calloc(ModelCount, sizeof(Model *));
		ModelColliderDefs.resize(ModelCount);	// Allocate space for the definitions

		for(int k = 0; k < ModelCount; k++) {
			MeshIds[ms[k]["id"]] = k;
			std::string MT = ms[k]["format"].template get<std::string>();
			std::string VDN = ms[k]["VD"].template get<std::string>();

			M[k] = new Model();
			if(MT[0] == 'A') {
				// init from asset file
				std::string AN = ms[k]["asset"].template get<std::string>();
				logs::debug("Getting from asset: '", AN, "'");
				int aId = AsIds[AN];
				logs::debug("Asset ID: ", aId);
				M[k]->initFromAsset(BP, VDIds[VDN], As[aId], ms[k]["model"], ms[k]["meshId"], ms[k]["node"]);
			} else {
				M[k]->init(BP, VDIds[VDN], ms[k]["model"], (MT[0] == 'O') ? OBJ : ((MT[0] == 'G') ? GLTF : MGCG));
			}

			// COLLIDERS
			//------------------------------------------------------------------------
			if (ms[k].contains("collider")) {
				ModelColliderDefs[k].hasCollider = true;
				nlohmann::json colData = ms[k]["collider"];

				// Recursive helper
				ParseColliderRecursive(colData, ModelColliderDefs[k], M, k);
			}
			//------------------------------------------------------------------------
		}

		// TEXTURES
		// TEXTURES
		nlohmann::json ts = js["textures"];
		TextureCount = ts.size();
		logs::debug("Textures count: ", TextureCount);

		T = (Texture **)calloc(TextureCount, sizeof(Texture *));
		for(int k = 0; k < TextureCount; k++) {
			TextureIds[ts[k]["id"]] = k;
			std::string TT = ts[k]["format"].template get<std::string>();

			T[k] = new Texture();
			if(TT[0] == 'C') {
				T[k]->init(BP, ts[k]["texture"]);
			} else if(TT[0] == 'D') {
				T[k]->init(BP, ts[k]["texture"], VK_FORMAT_R8G8B8A8_UNORM);
			} else {
				logs::warning("FORMAT UNKNOWN: ", TT);
			}
			logs::debug(ts[k]["id"].template get<std::string>(), "(", k, ") ", TT);
		}

		// INSTANCES TextureCount
		nlohmann::json pis = js["instances"];
		TechniqueInstanceCount = pis.size();
		logs::debug("Technique Instances count: ", TechniqueInstanceCount);
		TI = (TechniqueInstances *)calloc(TechniqueInstanceCount, sizeof(TechniqueInstances));
		InstanceCount = 0;

		for(int k = 0; k < TechniqueInstanceCount; k++) {
			std::string Pid = pis[k]["technique"].template get<std::string>();

			TI[k].T = TechniqueIds[Pid];
			nlohmann::json is = pis[k]["elements"];
			TI[k].InstanceCount = is.size();
			logs::debug("Technique: ", Pid, "(", k, "), Instances count: ", TI[k].InstanceCount);
			TI[k].I = (Instance *)calloc(TI[k].InstanceCount, sizeof(Instance));

			for(int j = 0; j < TI[k].InstanceCount; j++) {

				TI[k].I[j].id  = new std::string(is[j]["id"]);
				TI[k].I[j].Mid = MeshIds[is[j]["model"]];
				int NTextures = is[j]["texture"].size();
				if(NTextures != TI[k].T->Ntextures) {
					logs::error("Wrong number of textures!");
					exit(0);
				}
				TI[k].I[j].NTx = NTextures;
				TI[k].I[j].Tid = (int *)calloc(NTextures, sizeof(int));
				for(int h = 0; h < NTextures; h++) {
					TI[k].I[j].Tid[h] = TextureIds[is[j]["texture"][h]];
				}
				nlohmann::json TMjson = is[j]["transform"];
				if(TMjson.is_null()) {
					bool manualPos = false;

					glm::vec3 trT = glm::vec3(0.0f);
					glm::mat4 trR = glm::mat4(1.0f);
					glm::vec3 trS = glm::vec3(1.0f);
					nlohmann::json Tr_Tjson = is[j]["translate"];
					if(!Tr_Tjson.is_null()) {
						trT.x = Tr_Tjson[0];
						trT.y = Tr_Tjson[1];
						trT.z = Tr_Tjson[2];
						manualPos = true;
					}

					nlohmann::json Tr_REjson = is[j]["eulerAngles"];
					if(!Tr_REjson.is_null()) {
						trR = glm::rotate(glm::mat4(1.0f),
										  glm::radians((float)Tr_REjson[1]),
										  glm::vec3(0.0f,1.0f,0.0f)) *
							  glm::rotate(glm::mat4(1.0f),
										  glm::radians((float)Tr_REjson[0]),
										  glm::vec3(1.0f,0.0f,0.0f)) *
							  glm::rotate(glm::mat4(1.0f),
										  glm::radians((float)Tr_REjson[2]),
										  glm::vec3(0.0f,0.0f,1.0f));
						manualPos = true;
					} else {
						nlohmann::json Tr_RQjson = is[j]["quaternion"];
						if(!Tr_RQjson.is_null()) {
							glm::quat trQ = glm::quat(Tr_RQjson[0],
													  Tr_RQjson[1],
													  Tr_RQjson[2],
													  Tr_RQjson[3]);
							trR = glm::mat4(trQ);
							manualPos = true;
						}
					}

					nlohmann::json Tr_Sjson = is[j]["scale"];
					if(!Tr_Sjson.is_null()) {
						trS.x = Tr_Sjson[0];
						trS.y = Tr_Sjson[1];
						trS.z = Tr_Sjson[2];
						manualPos = true;
					}

					if(manualPos) {
						TI[k].I[j].Wm = glm::translate(glm::mat4(1.0f), trT) *
										trR *
										glm::scale(glm::mat4(1.0f), trS);
					} else {
						TI[k].I[j].Wm = M[TI[k].I[j].Mid]->Wm;
					}
				} else {
					float TMj[16];
					for(int h = 0; h < 16; h++) {TMj[h] = TMjson[h];}
					TI[k].I[j].Wm = glm::mat4(TMj[0],TMj[4],TMj[8],TMj[12],TMj[1],TMj[5],TMj[9],TMj[13],TMj[2],TMj[6],TMj[10],TMj[14],TMj[3],TMj[7],TMj[11],TMj[15]);
				}

				// INSTANCES Collider
				//------------------------------------------------------------------------
				int currentModelIndex = TI[k].I[j].Mid;
				if (ModelColliderDefs[currentModelIndex].hasCollider) {
					ColliderDef &def = ModelColliderDefs[currentModelIndex];

					// Recursive helper
					// Note: initial parentVisible flag is false, it will be the root "visible" flag to decide
					Collider *newCol = CreateColliderRecursive(def, GlobalColliders, ColShow, false);

					newCol->setWorldMatrix(TI[k].I[j].Wm);
					TI[k].I[j].C = newCol;
				}
				//------------------------------------------------------------------------

				TI[k].I[j].TIp = &TI[k];
				TI[k].I[j].D = (std::vector<DescriptorSetLayout *> **)calloc(sizeof(std::vector<DescriptorSetLayout *> *), Npasses);
				TI[k].I[j].NDs = (int *)calloc(sizeof(int), Npasses);
				for(int ipas = 0; ipas < Npasses; ipas++) {
					TI[k].I[j].D[ipas] = &TI[k].T->PT[ipas].P->D;
					TI[k].I[j].NDs[ipas] = TI[k].I[j].D[ipas]->size();
					BP->DPSZs.setsInPool += TI[k].I[j].NDs[ipas];
					for(int h = 0; h < TI[k].I[j].NDs[ipas]; h++) {
						DescriptorSetLayout *DSL = (*TI[k].I[j].D[ipas])[h];
						int DSLsize = DSL->Bindings.size();

						for (int l = 0; l < DSLsize; l++) {
							if(DSL->Bindings[l].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
								BP->DPSZs.uniformBlocksInPool += 1;
							} else {
								BP->DPSZs.texturesInPool += 1;
							}
						}
					}
				}
				InstanceCount++;
			}
		}

		logs::debug("Creating instances");
		I =  (Instance **)calloc(InstanceCount, sizeof(Instance *));

		int i = 0;
		for(int k = 0; k < TechniqueInstanceCount; k++) {
			for(int j = 0; j < TI[k].InstanceCount; j++) {
				I[i] = &TI[k].I[j];
				InstanceIds[*I[i]->id] = i;
				I[i]->Iid = i;

				i++;
			}
		}
		logs::debug(i, " instances created");
	return 0;
}


void Scene::pipelinesAndDescriptorSetsInit() {
	for(int i = 0; i < InstanceCount; i++) {

		I[i]->DS = (DescriptorSet ***)calloc(Npasses, sizeof(DescriptorSet **));
		for(int ipas = 0; ipas < Npasses; ipas++) {
			I[i]->DS[ipas] = (DescriptorSet **)calloc(I[i]->NDs[ipas], sizeof(DescriptorSet *));
			for(int j = 0; j < I[i]->NDs[ipas]; j++) {
				std::vector<VkDescriptorImageInfo> Tids = {};
				TechniqueRef *Tr = I[i]->TIp->T;
				int ntxs = Tr->PT[ipas].texDefs[j].size();
				Tids.resize(ntxs);
				for(int kt = 0; kt < ntxs; kt++) {
					if(Tr->PT[ipas].texDefs[j][kt].fromInstance) {
						Tids[kt] = T[I[i]->Tid[
									  Tr->PT[ipas].texDefs[j][kt].pos
								    ]]->getViewAndSampler();
					} else {
						Tids[kt] = Tr->PT[ipas].texDefs[j][kt].info;
					}
				}

				I[i]->DS[ipas][j] = new DescriptorSet();
				I[i]->DS[ipas][j]->init(BP, (*I[i]->D[ipas])[j], Tids);
			}
		}
	}
	logs::debug("Scene DS init Done");

	// Init and Update Collider Visualizer
	ColShow.pipelinesAndDescriptorSetsInit();
	ColShow.updateCommandBuffer();
}

void Scene::pipelinesAndDescriptorSetsCleanup() {
	// Cleanup datasets
	for(int i = 0; i < InstanceCount; i++) {
		for(int ipas = 0; ipas < Npasses; ipas++) {
			for(int j = 0; j < I[i]->NDs[ipas]; j++) {
				I[i]->DS[ipas][j]->cleanup();
				delete I[i]->DS[ipas][j];
			}
			free(I[i]->DS[ipas]);
		}
		free(I[i]->DS);
	}

	// Cleanup Collider Visualizer
	ColShow.pipelinesAndDescriptorSetsCleanup();
}

void Scene::localCleanup() {
	// Cleanup textures
	for(int i = 0; i < TextureCount; i++) {
		T[i]->cleanup();
		delete T[i];
	}
	free(T);

	// Cleanup models
	for(int i = 0; i < ModelCount; i++) {
		M[i]->cleanup();
		delete M[i];
	}
	free(M);

	// Cleanup Colliders
	for(auto *c : GlobalColliders) {
		delete c;
	}
	GlobalColliders.clear();
	ModelColliderDefs.clear();

	// Cleanup Colliders Visualizer
	ColShow.localCleanup();

	for(int i = 0; i < InstanceCount; i++) {
		delete I[i]->id;
		free(I[i]->Tid);
	}
	free(I);

	// To add: delete also the data structure relative to the pipeline
	for(int i = 0; i < TechniqueInstanceCount; i++) {
		free(TI[i].I);
	}
	free(TI);
}

void Scene::populateCommandBuffer(VkCommandBuffer commandBuffer, int passId, int currentImage) {
	if(passId >= Npasses) {
		logs::error("Scene Error: requested a pass too high in scene : ", passId, " >= ", Npasses);
		exit(0);
	}

	for(int k = 0; k < TechniqueInstanceCount; k++) {
		for(int i = 0; i < TI[k].InstanceCount; i++) {
			Pipeline *P = TI[k].T->PT[passId].P;
			if(P != nullptr) {
				P->bind(commandBuffer);

				M[TI[k].I[i].Mid]->bind(commandBuffer);
				for(int j = 0; j < TI[k].I[i].NDs[passId]; j++) {
					TI[k].I[i].DS[passId][j]->bind(commandBuffer, *P, j, currentImage);
				}
				vkCmdDrawIndexed(commandBuffer,
						static_cast<uint32_t>(M[TI[k].I[i].Mid]->indices.size()), 1, 0, 0, 0);
			}
		}
	}
}

// This function updates the shown colliders information to render them correctly
void Scene::updateColliderVisualizer(uint32_t currentImage, glm::mat4 ViewPrj) {
	ColShow.updateTransforms(currentImage, ViewPrj);
}

// This function forces a refresh of the visualized colliders (rebuild mesh and re-record command buffer)
void Scene::refreshColliderVisualizer()
{
	ColShow.refresh();
}

// Recursive helper to parse the JSON
void Scene::ParseColliderRecursive(nlohmann::json &node, ColliderDef &def, Model **M, int modelIndex) {
	// Shortcut for auto-fit supported colliders
    if(node.is_string()) {
        def.type = node.template get<std::string>();
    	if (def.type == "AABB")
    	{
    		def.type = "FitAABB";
    	}
    	else if (def.type == "OOBB")
    	{
    		def.type = "FitOOBB";
    	}
    	else
    	{
    		logs::error("Scene Error: shortcut for collider type: ", def.type, " not supported");
    		exit(0);
    	}
    }
	// Complete collider specification
    else if (node.is_object()) {
        def.type = node["type"].template get<std::string>();
        def.visible = node.value("visible", false);

        if(node.contains("params")) {
            for(auto& el : node["params"]) def.params.push_back(el);
        }

    	// If BVH, parse children recursively
        if(def.type == "BVH" && node.contains("children")) {
            for(auto& childNode : node["children"]) {
                ColliderDef childDef;
                childDef.hasCollider = true;
                ParseColliderRecursive(childNode, childDef, M, modelIndex);
                def.children.push_back(childDef);
            }
        }
    }

	// AABB Auto-Fit (by shortcut or by empty parameters)
    if (def.type == "FitAABB" || (def.type == "AABB" && def.params.empty())) {
         Collider tmp;
    	 tmp.fitAABB(M[modelIndex]);
         AABBextents ext = tmp.getExtents();
         def.params = {ext.xMin, ext.yMin, ext.zMin, ext.xMax, ext.yMax, ext.zMax};
         def.type = "AABB";
    }

	// OOBB Auto-Fit (by shortcut or by empty parameters)
    else if (def.type == "FitOOBB" || (def.type == "OOBB" && def.params.empty())) {
         Collider tmp;
    	 tmp.fitOOBB(M[modelIndex]);
         AABBextents ext = tmp.getExtents();
         def.params = {ext.xMin, ext.yMin, ext.zMin, ext.xMax, ext.yMax, ext.zMax};
         def.type = "OOBB";
    }
}

// Recursive helper to create the Colliders
Collider* Scene::CreateColliderRecursive(ColliderDef &def, std::vector<Collider *> &GlobalList, ColliderShow &Show, bool parentVisible) {
    Collider *newCol = new Collider();

    // Primitive creation
    if (def.type == "AABB") {
    	if (def.params.size() < 6)
    	{
    		logs::error("Scene Error: wrong number of parameters for collider type: ", def.type);
    		exit(0);
    	}
		newCol->initAABB(def.params[0], def.params[1], def.params[2], def.params[3], def.params[4], def.params[5]);
    }
    else if (def.type == "OOBB") {
    	if (def.params.size() < 6)
    	{
    		logs::error("Scene Error: wrong number of parameters for collider type: ", def.type);
    		exit(0);
    	}
		newCol->initOOBB(def.params[0], def.params[1], def.params[2], def.params[3], def.params[4], def.params[5]);
    }
    else if (def.type == "Sphere") {
    	if (def.params.size() < 4)
    	{
    		logs::error("Scene Error: wrong number of parameters for collider type: ", def.type);
    		exit(0);
    	}
		newCol->initSphere(def.params[0], def.params[1], def.params[2], def.params[3]);
    }
    else if (def.type == "Point") {
    	if (def.params.size() < 3)
    	{
    		logs::error("Scene Error: wrong number of parameters for collider type: ", def.type);
    		exit(0);
    	}
		newCol->initPoint(def.params[0], def.params[1], def.params[2]);
    }
    else if (def.type == "BVH") {
        std::vector<Collider *> childrenPtrs;
        bool isVisible = def.visible || parentVisible; // If BVH is visible, show the children

    	if (def.children.empty())
    	{
    		logs::error("Scene Error: collider type: ", def.type, " must have children");
    		exit(0);
    	}

        for(auto &childDef : def.children) {
        	// Recursively create the children
            Collider* childCol = CreateColliderRecursive(childDef, GlobalList, Show, isVisible);
            childrenPtrs.push_back(childCol);
        }

    	// Initialize the BVH with the list of the pointers to the children
        newCol->initBVH(childrenPtrs);
    }
    else
    {
    	logs::error("Scene Error: collider type: ", def.type, " not supported");
    	exit(0);
    }

	// Memory and Visibility management
    GlobalList.push_back(newCol);

	// If the collider is visible, add it to the visualizer
    if (def.visible) {
        Show.show(newCol);
    }

    return newCol;
}

void Scene::setColliderStroke(Collider* c, glm::vec4 color) {
	ColShow.setStroke(c, color);
}

#endif

// NOLINTEND(*)
// clang-format on
