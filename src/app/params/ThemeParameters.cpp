
//APP
#include <app/params/ThemeParameters.h>

//SOCLE
#include <ign/Exception.h>


namespace app{
namespace params{


	///
	///
	///
	ThemeParameters::ThemeParameters()
	{
		_initParameter( DB_CONF_FILE, "DB_CONF_FILE");
		_initParameter( COUNTRY_CODE_W, "COUNTRY_CODE_W");
		_initParameter( LANDMASK_TABLE, "LANDMASK_TABLE");
		_initParameter( REF_SCHEMA, "REF_SCHEMA");
		_initParameter( UP_SCHEMA, "UP_SCHEMA");
		_initParameter( THEME_W, "THEME_W");
		_initParameter( TABLE_W, "TABLE_W");
		_initParameter( TABLE_REF_SUFFIX, "TABLE_REF_SUFFIX");
		_initParameter( TABLE_UP_SUFFIX, "TABLE_UP_SUFFIX");
		_initParameter( TABLE_CD_SUFFIX, "TABLE_CD_SUFFIX");
		_initParameter( TABLE_UP_AREA_SUFFIX, "TABLE_UP_AREA_SUFFIX");
		_initParameter( ID_REF, "ID_REF");
		_initParameter( ID_UP, "ID_UP");
		_initParameter( GEOM_MATCH, "GEOM_MATCH");
		_initParameter( ATTR_MATCH, "ATTR_MATCH");

		_initParameter( UA_DIST_THRESHOLD, "UA_DIST_THRESHOLD");
		_initParameter( UA_DIST_BUFFER, "UA_DIST_BUFFER");
	}

	///
	///
	///
	ThemeParameters::~ThemeParameters()
	{
	}

	///
	///
	///
	std::string ThemeParameters::getClassName()const
	{
		return "app::params::ThemeParameters";
	}


}
}