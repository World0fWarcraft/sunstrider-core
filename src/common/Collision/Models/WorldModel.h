#ifndef _WORLDMODEL_H
#define _WORLDMODEL_H

#include "Define.h"
#include "WaterDefines.h"

#include <G3D/HashTrait.h>
#include <G3D/Vector3.h>
#include <G3D/AABox.h>
#include <G3D/Ray.h>
#include "BoundingIntervalHierarchy.h"

namespace VMAP
{
    class TreeNode;
    struct AreaInfo;
    struct LocationInfo;
    enum class ModelIgnoreFlags : uint32;

    class TC_COMMON_API MeshTriangle
    {
        public:
            MeshTriangle() : idx0(0), idx1(0), idx2(0) { }
            MeshTriangle(uint32 na, uint32 nb, uint32 nc): idx0(na), idx1(nb), idx2(nc) { }

            uint32 idx0;
            uint32 idx1;
            uint32 idx2;
    };

    class TC_COMMON_API WmoLiquid
    {
        public:
            WmoLiquid(uint32 width, uint32 height, const G3D::Vector3 &corner, uint32 type);
            WmoLiquid(const WmoLiquid &other);
            ~WmoLiquid();
            WmoLiquid& operator=(const WmoLiquid &other);
            bool GetLiquidHeight(const G3D::Vector3 &pos, float &liqHeight) const;
            LiquidType GetType() const { return iType; }
            float *GetHeightStorage() { return iHeight; }
            uint8 *GetFlagsStorage() { return iFlags; }
            uint32 GetFileSize();
            bool writeToFile(FILE* wf);
            static bool readFromFile(FILE* rf, WmoLiquid* &liquid);
            void getPosInfo(uint32 &tilesX, uint32 &tilesY, G3D::Vector3 &corner) const;
        private:
            WmoLiquid() : iTilesX(0), iTilesY(0), iCorner(), iType(LIQUID_TYPE_NO_WATER), iHeight(NULL), iFlags(NULL) { }
            uint32 iTilesX;       //!< number of tiles in x direction, each
            uint32 iTilesY;
            G3D::Vector3 iCorner; //!< the lower corner
            LiquidType iType;     //!< liquid type
            float *iHeight;       //!< (tilesX + 1)*(tilesY + 1) height values
            uint8 *iFlags;        //!< info if liquid tile is used
    };

    /*! holding additional info for WMO group files */
    class TC_COMMON_API GroupModel
    {
        public:
            GroupModel() : iBound(), iMogpFlags(0), iGroupWMOID(0), iLiquid(NULL) { }
            GroupModel(const GroupModel &other);
            GroupModel(uint32 mogpFlags, uint32 groupWMOID, const G3D::AABox &bound):
                        iBound(bound), iMogpFlags(mogpFlags), iGroupWMOID(groupWMOID), iLiquid(NULL) { }
            ~GroupModel() { delete iLiquid; }

            //! pass mesh data to object and create BIH. Passed vectors get get swapped with old geometry!
            void setMeshData(std::vector<G3D::Vector3> &vert, std::vector<MeshTriangle> &tri);
            void setLiquidData(WmoLiquid*& liquid) { iLiquid = liquid; liquid = NULL; }
            bool IntersectRay(const G3D::Ray &ray, float &distance, bool stopAtFirstHit) const;
            bool IsInsideObject(const G3D::Vector3 &pos, const G3D::Vector3 &down, float &z_dist) const;
            bool GetLiquidLevel(const G3D::Vector3 &pos, float &liqHeight) const;
            LiquidType GetLiquidType() const;
            LiquidType GetWMOLiquidType() const;
            bool writeToFile(FILE* wf);
            bool readFromFile(FILE* rf);
            const G3D::AABox& GetBound() const { return iBound; }
            uint32 GetMogpFlags() const { return iMogpFlags; }
            uint32 GetWmoID() const { return iGroupWMOID; }
            void getMeshData(std::vector<G3D::Vector3> &vertices, std::vector<MeshTriangle> &triangles, WmoLiquid* &liquid);
        protected:
            G3D::AABox iBound;
            uint32 iMogpFlags;// 0x8 outdor; 0x2000 indoor
            uint32 iGroupWMOID;
            std::vector<G3D::Vector3> vertices;
            std::vector<MeshTriangle> triangles;
            BIH meshTree;
            WmoLiquid* iLiquid;
    };
    /*! Holds a model (converted M2 or WMO) in its original coordinate space */
    class TC_COMMON_API WorldModel
    {
        public:
            WorldModel(): RootWMOID(0), Flags(0) { }

            //! pass group models to WorldModel and create BIH. Passed vector is swapped with old geometry!
            void setGroupModels(std::vector<GroupModel> &models);
            void setRootWmoID(uint32 id) { RootWMOID = id; }
            bool IntersectRay(const G3D::Ray &ray, float &distance, bool stopAtFirstHit, ModelIgnoreFlags ignoreFlags) const;
            bool IntersectPoint(const G3D::Vector3 &p, const G3D::Vector3 &down, float &dist, AreaInfo &info) const;
            bool GetLocationInfo(const G3D::Vector3 &p, const G3D::Vector3 &down, float &dist, LocationInfo &info) const;
            bool writeFile(const std::string &filename);
            bool readFile(const std::string &filename);
            void getGroupModels(std::vector<GroupModel> &groupModels);
            uint32 Flags;
        protected:
            uint32 RootWMOID;
            std::vector<GroupModel> groupModels;
            BIH groupTree;
    };
} // namespace VMAP

#endif // _WORLDMODEL_H
