#ifndef _APP_CALCUL_UPAREACREATIONOP_H_
#define _APP_CALCUL_UPAREACREATIONOP_H_


//SOCLE
#include <ign/feature/sql/FeatureStorePostgis.h>

//EPG
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>


namespace app{
namespace calcul{

	class UpAreaCreationOp {

	public:

	
        /// @brief 
        /// @param countryCode 
        /// @param verbose 
        UpAreaCreationOp(
            std::string countryCode,
            bool verbose
        );

        /// @brief 
        ~UpAreaCreationOp();


		/// \brief
		static void Compute(
			std::string countryCode, 
			bool verbose
		);


	private:
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsProd;
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsRef;
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsUp;
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsCd;
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsBorder;
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsUpArea;
		//--
		epg::log::EpgLogger*                                     _logger;
		//--
		epg::log::ShapeLogger*                                   _shapeLogger;
		//--
		std::string                                              _countryCode;
		//--
		bool                                                     _verbose;

	private:

		//--
		void _init();

		//--
		void _compute() const;

		//--
		void _createAreas(
            ign::feature::FeatureFilter const& filter,
            ign::feature::sql::FeatureStorePostgis* fs,
            std::set<std::string> const& sCd,
            std::set<std::string> & sTreated,
            ign::geometry::GeometryPtr & resultGeomPtr
        ) const;

    };

}
}

#endif