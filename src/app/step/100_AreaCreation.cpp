#include <app/step/100_AreaCreation.h>

//EPG
#include <epg/Context.h>
#include <epg/log/ScopeLogger.h>

//APP
#include <app/params/ThemeParameters.h>
#include <app/calcul/UpAreaCreationOp.h>


namespace app {
namespace step {

	///
	///
	///
	void AreaCreation::init()
	{
		// addWorkingEntity(AREA_TABLE_INIT);
	}

	///
	///
	///
	void AreaCreation::onCompute( bool verbose = false )
	{
		app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
		std::string countryCodeW = themeParameters->getParameter(COUNTRY_CODE_W).getValue().toString();

		app::calcul::UpAreaCreationOp::Compute(countryCodeW, verbose);
	}

}
}
