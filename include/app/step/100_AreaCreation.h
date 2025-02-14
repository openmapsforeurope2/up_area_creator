#ifndef _APP_STEP_AREACREATION_H_
#define _APP_STEP_AREACREATION_H_

#include <epg/step/StepBase.h>
#include <app/params/ThemeParameters.h>

namespace app{
namespace step{

	class AreaCreation : public epg::step::StepBase< app::params::ThemeParametersS > {

	public:

		/// \brief
		int getCode() { return 100; };

		/// \brief
		std::string getName() { return "AreaCreation"; };

		/// \brief
		void onCompute( bool );

		/// \brief
		void init();

	};

}
}

#endif