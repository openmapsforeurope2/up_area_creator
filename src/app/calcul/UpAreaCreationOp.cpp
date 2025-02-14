// APP
#include <app/calcul/UpAreaCreationOp.h>
#include <app/params/ThemeParameters.h>

// BOOST
#include <boost/progress.hpp>

// EPG
#include <epg/Context.h>
#include <epg/params/EpgParameters.h>
#include <epg/sql/tools/numFeatures.h>
#include <epg/sql/DataBaseManager.h>
#include <epg/tools/StringTools.h>
#include <epg/tools/TimeTools.h>
#include <epg/tools/FilterTools.h>
// #include <epg/tools/geometry/simplifyLineString.h>
#include <epg/tools/geometry/getArea.h>

// IGN
#include <ign/geometry/algorithm/SimplifyOpGeos.h>


namespace app
{
    namespace calcul
    {
        ///
        ///
        ///
        UpAreaCreationOp::UpAreaCreationOp(
            std::string countryCode,
            bool verbose
        ) : 
            _countryCode(countryCode),
            _verbose(verbose)
        {
            _init();
        }

        ///
        ///
        ///
        UpAreaCreationOp::~UpAreaCreationOp()
        {

            // _shapeLogger->closeShape("ps_cutting_ls");
        }

        ///
        ///
        ///
        void UpAreaCreationOp::Compute(
			std::string countryCode, 
			bool verbose
		) {
            UpAreaCreationOp op(countryCode, verbose);
            op._compute();
        }

        ///
        ///
        ///
        void UpAreaCreationOp::_init()
        {
            //--
            _logger = epg::log::EpgLoggerS::getInstance();
            _logger->log(epg::log::INFO, "[START] initialization: " + epg::tools::TimeTools::getTime());

            //--
            _shapeLogger = epg::log::ShapeLoggerS::getInstance();
            // _shapeLogger->addShape("ps_cutting_ls", epg::log::ShapeLogger::POLYGON);

            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const idName = epgParams.getValue(ID).toString();
            std::string const geomName = epgParams.getValue(GEOM).toString();
            std::string const countryCodeName = epgParams.getValue(COUNTRY_CODE).toString();
            std::string const borderTableName = epgParams.getValue(TARGET_BOUNDARY_TABLE).toString();
            
            // app parameters
            params::ThemeParameters *themeParameters = params::ThemeParametersS::getInstance();
            std::string const themeName = themeParameters->getValue(THEME_W).toString();
            std::string const tableName = themeParameters->getValue(TABLE_W).toString();
            std::string refTableName = tableName + themeParameters->getValue(TABLE_REF_SUFFIX).toString();
            std::string upTableName = tableName + themeParameters->getValue(TABLE_UP_SUFFIX).toString();
            std::string cdTableName = tableName + themeParameters->getValue(TABLE_CD_SUFFIX).toString();
            std::string upAreaTableName = tableName + themeParameters->getValue(TABLE_UP_AREA_SUFFIX).toString();

            if ( !context->getDataBaseManager().tableExists(upAreaTableName) ) {
				std::ostringstream ss;
				ss << "CREATE TABLE " << upAreaTableName << "("
                    << geomName << " geometry(multipolygon, 3035),"
					<< idName << " uuid DEFAULT gen_random_uuid(),"
					<< countryCodeName << " character varying(5)"
					<< ");"
                    << "CREATE INDEX " << upAreaTableName +"_"+countryCodeName+"_idx ON " << upAreaTableName << " USING btree ("<< countryCodeName <<");"
                    << "CREATE INDEX " << upAreaTableName +"_"+geomName+"_idx ON " << upAreaTableName << " USING gist ("<< geomName <<");";

				context->getDataBaseManager().getConnection()->update(ss.str());

			} else {
                std::ostringstream ss;
                ss << "DELETE FROM " << upAreaTableName << " WHERE " << countryCodeName << " = '" << _countryCode << "';";
                context->getDataBaseManager().getConnection()->update(ss.str());
            }

            //--
            // _fsProd = context->getDataBaseManager().getFeatureStore(tableName, idName, geomName);
            _fsRef = context->getDataBaseManager().getFeatureStore(refTableName, idName, geomName);
            _fsUp = context->getDataBaseManager().getFeatureStore(upTableName, idName, geomName);
            _fsCd = context->getDataBaseManager().getFeatureStore(cdTableName, idName, geomName);
            _fsBorder = context->getDataBaseManager().getFeatureStore(borderTableName, idName, geomName);
            _fsUpArea = context->getDataBaseManager().getFeatureStore(upAreaTableName, idName, geomName);

            //--
            _logger->log(epg::log::INFO, "[END] initialization: " + epg::tools::TimeTools::getTime());
        };

        ///
        ///
        ///
        void UpAreaCreationOp::_compute() const {
            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const geomName = epgParams.getValue(GEOM).toString();
            std::string const countryCodeName = epgParams.getValue(COUNTRY_CODE).toString();
            std::string const boundaryTypeName = epgParams.getValue(BOUNDARY_TYPE).toString();
            std::string const interBoundaryTypeValue = epgParams.getValue(TYPE_INTERNATIONAL_BOUNDARY).toString();

            // app parameters
            params::ThemeParameters *themeParameters = params::ThemeParametersS::getInstance();
            std::string const idRefName = themeParameters->getValue(ID_REF).toString();
            std::string const idUpName = themeParameters->getValue(ID_UP).toString();
            std::string const distThreshold = themeParameters->getValue(UA_DIST_THRESHOLD).toString();


            //load idUp and idRef
            std::set<std::string> sIdUp;
            std::set<std::string> sIdRef;
            ign::feature::FeatureFilter cdFilter;
            ign::feature::FeatureIteratorPtr itCd = _fsCd->getFeatures(cdFilter);
            while (itCd->hasNext())
            {
                ign::feature::Feature const& fCd = itCd->next();
                ign::data::Variant const& idRef = fCd.getAttribute(idRefName);
                ign::data::Variant const& idUp = fCd.getAttribute(idUpName);

                if (!idRef.isNull()) sIdRef.insert(idRef.toString());
                if (!idUp.isNull()) sIdUp.insert(idUp.toString());
            }

            ign::feature::FeatureFilter filterBorder(countryCodeName+" LIKE '%"+_countryCode+"%' AND ("+countryCodeName+" LIKE '%#%' OR "+boundaryTypeName+"='"+interBoundaryTypeValue+"')");

            int numBorders = epg::sql::tools::numFeatures(*_fsBorder, filterBorder);
            size_t countBorder = 0;
            std::set<std::string> sTreatedProd;
            std::set<std::string> sTreatedRef;
            std::set<std::string> sTreatedUp;
            ign::feature::FeatureIteratorPtr itBorder = _fsBorder->getFeatures(filterBorder);
            while (itBorder->hasNext())
            {
                std::cout << "processing border [" << ++countBorder << "/" << numBorders << "]";

                ign::feature::Feature const& fBorder = itBorder->next();
                ign::geometry::Geometry const& geomBorder = fBorder.getGeometry();

                ign::feature::FeatureFilter filter("ST_INTERSECTS("+geomName+", ST_SetSRID(ST_Buffer(ST_GeomFromText('"+ geomBorder.toString() +"'),"+distThreshold+"),3035))");

                // _createAreas(filter, _fsProd, sIdRef, sTreatedProd);
                ign::geometry::GeometryPtr resultGeomPtr(new ign::geometry::MultiPolygon());
                _createAreas(filter, _fsRef, sIdRef, sTreatedRef, resultGeomPtr);
                _createAreas(filter, _fsUp, sIdUp, sTreatedUp, resultGeomPtr);

                if(resultGeomPtr && !resultGeomPtr->isEmpty()) {
                    ign::feature::Feature fArea = _fsUpArea->newFeature();
                    ign::geometry::GeometryPtr simplifiedGeomPtr(new ign::geometry::MultiPolygon());

                    //
                    ign::geometry::MultiPolygon & mp = resultGeomPtr->asMultiPolygon();
                    for (size_t i = 0 ; i < mp.numGeometries() ; ++i) {
                        ign::geometry::Polygon & poly = mp.polygonN(i);
                        // _logger->log(epg::log::DEBUG, poly.toString());

                        // if ( !poly.intersects(ign::geometry::Point(4024264.5883254, 2964606.0199504))) continue;

                        // ign::geometry::GeometryPtr simplyfiedExtRing( ign::geometry::algorithm::SimplifyOpGeos::DouglasPeuckerSimplify( poly.exteriorRing(), 2 ) );

                        // simplifiedGeomPtr.reset(simplifiedGeomPtr->Union(ign::geometry::Polygon(simplyfiedExtRing->asLineString())));
                        simplifiedGeomPtr.reset(simplifiedGeomPtr->Union(ign::geometry::Polygon(poly.exteriorRing())));

                        // ign::geometry::Polygon newPoly(poly.exteriorRing());

                        for( int j = poly.numInteriorRing()-1 ; j >= 0 ; --j ) {
                            double area = epg::tools::geometry::getArea(poly.interiorRingN(j));
                            if( std::abs(area) < 10000 )
                                poly.removeInteriorRingN(j);
                            else {
                                // ign::geometry::GeometryPtr simplyfiedRing( ign::geometry::algorithm::SimplifyOpGeos::DouglasPeuckerSimplify( poly.interiorRingN(j), 2 ) );
                                // simplifiedGeomPtr.reset(simplifiedGeomPtr->Difference(ign::geometry::Polygon(simplyfiedRing->asLineString())));
                                simplifiedGeomPtr.reset(simplifiedGeomPtr->Difference(poly.interiorRingN(j)));
                            }
                        }
                    }

                    fArea.setGeometry(*simplifiedGeomPtr);
                    fArea.setAttribute(countryCodeName, ign::data::String(_countryCode));
                    _fsUpArea->createFeature(fArea);
                }
            }
        }

        ///
        ///
        ///
        void UpAreaCreationOp::_createAreas(
            ign::feature::FeatureFilter const& filter,
            ign::feature::sql::FeatureStorePostgis* fs,
            std::set<std::string> const& sCd,
            std::set<std::string> & sTreated,
            ign::geometry::GeometryPtr & resultGeomPtr
        ) const {
            //--
            epg::Context *context = epg::ContextS::getInstance();
            
            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const countryCodeName = epgParams.getValue(COUNTRY_CODE).toString();

            // app parameters
            params::ThemeParameters *themeParameters = params::ThemeParametersS::getInstance();
            double const distBuffer = themeParameters->getValue(UA_DIST_BUFFER).toDouble();

            int nFeats = epg::sql::tools::numFeatures(*fs, filter);
            boost::progress_display display(nFeats, std::cout, "[ creating areas % complete ]\n");

            ign::feature::FeatureIteratorPtr itFeat = fs->getFeatures(filter);
            while (itFeat->hasNext())
            {
                ++display;

                ign::feature::Feature const& feat = itFeat->next();
                std::string const& featId = feat.getId();

                if( sCd.find(featId) == sCd.end() ) continue;
                if( sTreated.find(featId) != sTreated.end() ) continue;
                sTreated.insert(featId);

                // ign::feature::Feature fArea = _fsUpArea->newFeature();
                // fArea.setGeometry(feat.getGeometry().buffer(distBuffer));
                // fArea.setAttribute(countryCodeName, ign::data::String(_countryCode));
                // _fsUpArea->createFeature(fArea);

                resultGeomPtr.reset(resultGeomPtr->Union(*feat.getGeometry().buffer(distBuffer)));
            }
        }
    }
}
