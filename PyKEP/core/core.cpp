/*****************************************************************************
 *   Copyright (C) 2004-2009 The PaGMO development team,                     *
 *   Advanced Concepts Team (ACT), European Space Agency (ESA)               *
 *   http://apps.sourceforge.net/mediawiki/pagmo                             *
 *   http://apps.sourceforge.net/mediawiki/pagmo/index.php?title=Developers  *
 *   http://apps.sourceforge.net/mediawiki/pagmo/index.php?title=Credits     *
 *   act@esa.int                                                             *
 *                                                                           *
 *   This program is free software; you can redistribute it and/or modify    *
 *   it under the terms of the GNU General Public License as published by    *
 *   the Free Software Foundation; either version 3 of the License, or       *
 *   (at your option) any later version.                                     *
 *                                                                           *
 *   This program is distributed in the hope that it will be useful,         *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *   GNU General Public License for more details.                            *
 *                                                                           *
 *   You should have received a copy of the GNU General Public License       *
 *   along with this program; if not, write to the                           *
 *   Free Software Foundation, Inc.,                                         *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.               *
 *****************************************************************************/

#include <boost/python.hpp>
#include <boost/python/class.hpp>
#include <boost/python/def.hpp>
#include <boost/python/copy_const_reference.hpp>
#include <boost/python/enum.hpp>
#include <boost/python/module.hpp>
#include <boost/python/operators.hpp>
#include <boost/python/register_ptr_to_python.hpp>
#include <boost/python/converter/registry.hpp>
#include <boost/python/self.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/overloads.hpp>
#include <boost/utility.hpp>

#include "../utils.h"
#include "../../src/keplerian_toolbox.h"
#include "../boost_python_container_conversions.h"

using namespace boost::python;

static inline tuple closest_distance_wrapper(const kep_toolbox::array3D& r0, const kep_toolbox::array3D& v0, const kep_toolbox::array3D& r1, const kep_toolbox::array3D& v1, const double &mu)
{
	double min_d, ra;
	kep_toolbox::closest_distance(min_d,ra,r0, v0, r1, v1, mu);
	return boost::python::make_tuple(min_d,ra);
}

static inline tuple planet_get_eph(const kep_toolbox::planet &p, const kep_toolbox::epoch &when)
{
	kep_toolbox::array3D r, v;
	p.get_eph(when,r,v);
	return boost::python::make_tuple(r,v);
}

static inline tuple propagate_lagrangian_wrapper(const kep_toolbox::array3D &r0, const kep_toolbox::array3D &v0, const double &t, const double &mu)
{
	kep_toolbox::array3D r(r0), v(v0);
	kep_toolbox::propagate_lagrangian_u(r,v,t,mu);
	return boost::python::make_tuple(r,v);
}

static inline tuple propagate_taylor_wrapper(const kep_toolbox::array3D &r0, const kep_toolbox::array3D &v0, const double &m0, const kep_toolbox::array3D &u, const double &t, const double &mu, const double &veff, const int &log10tolerance, const int &log10rtolerance)
{
	kep_toolbox::array3D r(r0), v(v0);
	double m(m0);
	kep_toolbox::propagate_taylor(r,v,m,u,t,mu,veff,log10tolerance,log10rtolerance);
	return boost::python::make_tuple(kep_toolbox::array3D(r),kep_toolbox::array3D(v),double(m));
}

static inline tuple propagate_taylor_s_wrapper(const kep_toolbox::array3D &r0, const kep_toolbox::array3D &v0, const double &m0, const kep_toolbox::array3D &u, const double &s, const double &mu=1.0, const double &veff=1.0, const double &c=1.0, const double &alpha=1.5, const int &log10tolerance=-10, const int &log10rtolerance=-10)
{
	kep_toolbox::array3D r(r0), v(v0);
	double m(m0); double dt(0);
	kep_toolbox::propagate_taylor_s(r,v,m,dt,u,s,mu,veff,c,alpha,log10tolerance,log10rtolerance);
	return boost::python::make_tuple(kep_toolbox::array3D(r),kep_toolbox::array3D(v),double(m),double(dt));
}

static inline tuple fb_con_wrapper(const kep_toolbox::array3D &vin_rel, const kep_toolbox::array3D &vout_rel, const kep_toolbox::planet &pl)
{
	double eq(0), ineq(0);
	kep_toolbox::fb_con(eq, ineq, vin_rel, vout_rel, pl);
	return boost::python::make_tuple(double(eq),double(ineq));
}

static inline kep_toolbox::array3D fb_prop_wrapper(const kep_toolbox::array3D& v_in, const kep_toolbox::array3D& v_pla, const double &rp, const double& beta, const double& mu)
{
	kep_toolbox::array3D retval;
	kep_toolbox::fb_prop(retval,v_in,v_pla,rp,beta,mu);
	return retval;
}

static inline double fb_vel_wrapper(const kep_toolbox::array3D &vin_rel, const kep_toolbox::array3D &vout_rel, const kep_toolbox::planet &pl)
{
	double dV(0);
	kep_toolbox::fb_vel(dV, vin_rel, vout_rel, pl);
	return dV;
}

#define get_constant(arg) \
static inline double get_##arg() \
{ \
	return ASTRO_##arg; \
}

get_constant(AU);
get_constant(JR);
get_constant(MU_SUN);
get_constant(EARTH_VELOCITY);
get_constant(DEG2RAD);
get_constant(RAD2DEG);
get_constant(DAY2SEC);
get_constant(SEC2DAY);
get_constant(DAY2YEAR);
get_constant(G0);

#define PYKEP_REGISTER_CONVERTER(T,policy) \
{\
   boost::python::type_info info = boost::python::type_id<T >(); \
   const boost::python::converter::registration* reg = boost::python::converter::registry::query(info); \
   if (reg == NULL)  \
   {\
	to_tuple_mapping<T >();\
	from_python_sequence<T,policy>();\
   }\
   else if ((*reg).m_to_python == NULL)\
   {\
	to_tuple_mapping<T >();\
	from_python_sequence<T,policy>();\
   }\
}

#define EXPOSE_CONSTANT(arg) \
def("_get_"#arg,&get_##arg);

typedef boost::array<double,8> array8D;
typedef boost::array<double,11> array11D;

BOOST_PYTHON_MODULE(_core) {
	// Disable docstring c++ signature to allow sphinx autodoc to work properly
	docstring_options doc_options;
	doc_options.disable_signatures();

	//Register std converters to python lists if not already registered by some other module
	PYKEP_REGISTER_CONVERTER(std::vector<double >, variable_capacity_policy)
	PYKEP_REGISTER_CONVERTER(kep_toolbox::array3D,fixed_size_policy)
	PYKEP_REGISTER_CONVERTER(kep_toolbox::array6D,fixed_size_policy)
	PYKEP_REGISTER_CONVERTER(kep_toolbox::array7D,fixed_size_policy)
	PYKEP_REGISTER_CONVERTER(array8D,fixed_size_policy)
	PYKEP_REGISTER_CONVERTER(array11D,fixed_size_policy)
	PYKEP_REGISTER_CONVERTER(std::vector<kep_toolbox::array3D>, variable_capacity_policy)
	PYKEP_REGISTER_CONVERTER(std::vector<array8D>, variable_capacity_policy)
	PYKEP_REGISTER_CONVERTER(std::vector<array11D>,variable_capacity_policy)

	// Expose the astrodynamical constants.
	EXPOSE_CONSTANT(AU);
	EXPOSE_CONSTANT(JR);
	EXPOSE_CONSTANT(MU_SUN);
	EXPOSE_CONSTANT(EARTH_VELOCITY);
	EXPOSE_CONSTANT(DEG2RAD);
	EXPOSE_CONSTANT(RAD2DEG);
	EXPOSE_CONSTANT(DAY2SEC);
	EXPOSE_CONSTANT(SEC2DAY);
	EXPOSE_CONSTANT(DAY2YEAR);
	EXPOSE_CONSTANT(G0);

	// Exposing enums
	enum_<kep_toolbox::epoch::type>("_epoch_type",
			"Defines a julian date type exposing the corresponding c++ enum\n\n"
			"One of\n\n"
			"* PyKEP.epoch.epoch_type.JD\n"
			"* PyKEP.epoch.epoch_type.MJD\n"
			"* PyKEP.epoch.epoch_type.MJD2000\n"
		)
		.value("MJD", kep_toolbox::epoch::MJD)
		.value("MJD2000", kep_toolbox::epoch::MJD2000)
		.value("JD", kep_toolbox::epoch::JD);

	// Epoch class
	class_<kep_toolbox::epoch>("epoch","Represents a precise point in time. The boost::posix_time_ptime classes are used to handle the conversion to and from string",
		init<const double &,optional<kep_toolbox::epoch::type> >(
			"PyKEP.epoch(jd[, jd_type=epoch.epoch_type.MJD2000])\n\n"
			"- jd: a julian date\n"
			"- jd_type: julian date type, see :py:class:`PyKEP.epoch.epoch_type`\n\n"
			"Examples::\n\n"
			"  e1 = epoch(0)\n"
			"  e1 = epoch(0,epoch.epoch_type.MJD2000)\n"
			"  e2 = epoch(54333, epoch.epoch_type.MJD)\n"
		))
		.add_property("jd",&kep_toolbox::epoch::jd,
			"Returns the Julian Date\n\n"
			"Example::\n\n"
			"  jd = e.jd"
		)
		.add_property("mjd",&kep_toolbox::epoch::mjd,
			"Returns the Modified Julian Date\n\n"
			"Example::\n\n"
			"  jd = e.mjd"
		)
		.add_property("mjd2000",&kep_toolbox::epoch::mjd2000,
			"Returns the Modifeid Julian Date 2000\n\n"
			"Example::\n\n"
			"  jd = e.mjd2000"
		)
		.def(repr(self))
		.def_pickle(generic_pickle_suite<kep_toolbox::epoch>())
		.def(init<>());

	// Epoch constructors helpers
	def("epoch_from_string",&kep_toolbox::epoch_from_string,
		"PyKEP.epoch_from_string(s)\n\n"
		"- s: string containing a date in the format 'YYYY-MM-DD HH:MM:SS'"
		"Returns a :py:class:`PyKEP.epoch` object constructed from a from a delimited string containing a date."
		"Excess digits in fractional seconds will be dropped. Ex: '1:02:03.123456999' => '1:02:03.123456'."
		"This behavior depends on the precision defined in astro_constant.h used to compile.\n\n"
		"NOTE: The function is based on the corresponding `boost date_time library function <http://www.boost.org/doc/libs/1_44_0/doc/html/date_time/posix_time.html#ptime_from_string>`_\n\n"
		"Example::\n\n"
		"  e = PyKEP.epoch_from_string('2002-01-20 23:59:54.003')"
	);

	def("epoch_from_iso_string",&kep_toolbox::epoch_from_iso_string,
			"PyKEP.epoch_from_iso_string(s)\n\n"
		"- s: string containing a date in the iso format 'YYYYMMDDTHHMMSS'"
		"Returns a :py:class:`PyKEP.epoch` object constructed from a from a non delimited iso form string containing a date.\n\n"
		"NOTE: The function is based on the corresponding `boost date_time library function <http://www.boost.org/doc/libs/1_44_0/doc/html/date_time/posix_time.html#ptime_from_string>`_\n\n"
		"Example::\n\n"
		"  e = PyKEP.epoch_from_iso_string('20020120T235954')"
	);

	// Base planet class.

	// These serve to allow boost python to resolve correctly the overloaded function get_elements
	typedef kep_toolbox::array6D (kep_toolbox::planet::*element_getter)() const;
	//typedef kep_toolbox::array6D (kep_toolbox::planet::*element_getter_epoch)(const kep_toolbox::epoch &) const;

	class_<kep_toolbox::planet>("planet","A planet ... contains the ephemerides calculations",
		init<const kep_toolbox::epoch&, const kep_toolbox::array6D&, const double& , const double &, const double &, const double &, optional<const std::string &> >(
			"PyKEP.planet(when,orbital_elements, mu_central_body, mu_self,radius, safe_radius [, name = 'unknown'])\n\n"
			"- when: a :py:class:`PyKEP.epoch` indicating the orbital elements epoch\n"
			"- orbital_elements: a sequence of six containing a,e,i,W,w,M (SI units, i.e. meters and radiants)\n"
			"- mu_central_body: gravity parameter of the central body (SI units, i.e. m^2/s^3)\n"
			"- mu_self: gravity parameter of the planet (SI units, i.e. m^2/s^3)\n"
			"- radius: body radius (SI units, i.e. meters)\n"
			"- safe_radius: mimimual radius that is safe during a fly-by of the planet (SI units, i.e. m)\n"
			"- name: body name\n\n"
			"NOTE: use the derived classes :py:class:`PyKEP.planet_ss`, :py:class:`PyKEP.planet_mpcorb` to instantiate common objects"
			"Example::\n\n"
			"  earth = planet(epoch(54000,epoch.epoch_type.MJD),(9.9998805e-01 * AU, 1.6716812e-02, 8.8543531e-04 * DEG2RAD, 1.7540648e+02 * DEG2RAD, 2.8761578e+02 * DEG2RAD, 2.5760684e+02 * DEG2RAD), MU_SUN, 398600.4418e9, 6378000, 6900000,  'Earth'"
		))
		.def("eph",&planet_get_eph,
			"PyKEP.planet.eph(when)\n\n"
			"- when: a :py:class:`PyKEP.epoch` indicating the epoch at which the ephemerides are needed\n\n"
			"Retuns a tuple containing the planet position and velocity in SI units\n\n"
			"Example::\n\n"
			"  r,v = earth.eph(epoch(5433))"
		)
		.add_property("safe_radius",&kep_toolbox::planet::get_safe_radius, &kep_toolbox::planet::set_safe_radius,
			"The planet safe radius (distance at which it is safe for spacecraft to fly-by)\n\n"
			"Example::\n\n"
			"  Rs = earth.safe_radius"
			"  earth.safe_radius = 1.05"
			)
		.add_property("mu_self",&kep_toolbox::planet::get_mu_self,
			"The planet radius\n\n"
			"Example::\n\n"
			"  mu_pla = earth.mu_self"
			)
		.add_property("mu_central_body",&kep_toolbox::planet::get_mu_central_body,
			"The planet radius\n\n"
			"Example::\n\n"
			"  mu = earth.mu_central_body"
			)
		.add_property("name",&kep_toolbox::planet::get_name,
			"The planet Name\n\n"
			"Example::\n\n"
			"  name = earth.name()"
		)
		.add_property("orbital_elements",element_getter(&kep_toolbox::planet::get_elements),
			"A tuple containing the six orbital elements a,e,i,W,w,M at the reference epoch (SI units used)\n\n"
			"Example::\n\n"
			"  elem = earth.orbital_elements"
		)
		.add_property("ref_epoch",&kep_toolbox::planet::get_ref_epoch,
			"The reference epoch for M in :py:class:`PyKEP.planet.orbital_elements`\n\n"
			"Example::\n\n"
			"  epoch = earth.ref_epoch"
		)
		.add_property("radius",&kep_toolbox::planet::get_radius,
			"The planet radius\n\n"
			"Example::\n\n"
			"  R = earth.radius"
			)
		.add_property("period",&kep_toolbox::planet::compute_period,
			"The planet orbital period\n\n"
			"Example::\n\n"
			"  T = earth.period"
			)
		.def(repr(self))
		.def_pickle(generic_pickle_suite<kep_toolbox::planet>())
		.def(init<>());

	// A solar system planet
	class_<kep_toolbox::planet_ss,bases<kep_toolbox::planet> >("planet_ss","A planet from the solar system",
		init<std::string>(
			"PyKEP.planet_ss.eph(which)\n\n"
			"- which: string containing the common planet name (e.g. 'earth')\n\n"
			"Example::\n\n"
			"  earth = planet_ss('earth')"
		))
		.def("__copy__", &Py_copy_from_ctor<kep_toolbox::planet_ss>)
		.def("__deepcopy__", &Py_deepcopy_from_ctor<kep_toolbox::planet_ss>)
		.def("cpp_loads", &py_cpp_loads<kep_toolbox::planet_ss>)
		.def("cpp_dumps", &py_cpp_dumps<kep_toolbox::planet_ss>)
		.def_pickle(generic_pickle_suite<kep_toolbox::planet_ss>())
		.def(init<>());

	// A Jupiter system moon
	class_<kep_toolbox::planet_js,bases<kep_toolbox::planet> >("planet_js","A moon from the Jupiter system",
		init<std::string>(
			"PyKEP.planet_js.eph(which)\n\n"
			"- which: string containing the common planet name (e.g. 'io', 'europa', 'callisto' or 'ganymede')\n\n"
			"Example::\n\n"
			"  io = planet_js('io')"
		))
		.def_pickle(generic_pickle_suite<kep_toolbox::planet_js>())
		.def(init<>());

	// A planet from the MPCORB database
	class_<kep_toolbox::planet_mpcorb,bases<kep_toolbox::planet> >("planet_mpcorb","A planet from the mpcorb database",
		init<std::string>(
			"PyKEP.planet_mpcorb.eph(line)\n\n"
			"- line: a line from the MPCORB database file\n\n"
			"Example::\n\n"
			"  apophis = planet_mpcorb('99942   19.2   0.15 K107N 202.49545  126.41859  204.43202    3.33173  0.1911104  1.11267324   0.9223398  1 MPO164109  1397   2 2004-2008 0.40 M-v 3Eh MPCAPO     C802  (99942) Apophis            20080109')"
		))
		.add_property("H",&kep_toolbox::planet_mpcorb::get_H,
			"The asteroid absolute magnitude. This is assuming an albedo of 0.25 and using the formula at www.physics.sfasu.edu/astro/asteroids/sizemagnitude.html\n"
			"Example::\n\n"
			"  H = apophis.H"
			)
		.add_property("n_observations",&kep_toolbox::planet_mpcorb::get_n_observations,
			"Number of observations made on the asteroid\n"
			"Example::\n\n"
			"  R = apophis.n_observations"
			)
		.add_property("n_oppositions",&kep_toolbox::planet_mpcorb::get_n_oppositions,
			"The planet radius\n"
			"Example::\n\n"
			"  R = apophis.n_oppositions"
			)
		.add_property("year_of_discovery",&kep_toolbox::planet_mpcorb::get_year_of_discovery,
			"The year the asteroid was first discovered. In case the aastroid has been observed only once (n_observations), this number is, instead, the Arc Length in days\n"
			"Example::\n\n"
			"  R = apophis.year_of_discovery"
			)
		.def_pickle(generic_pickle_suite<kep_toolbox::planet_mpcorb>())
		.def(init<>());

	// A planet from the gtoc5 problem
	class_<kep_toolbox::asteroid_gtoc5,bases<kep_toolbox::planet> >("planet_gtoc5",
		init<const int &>(
			"PyKEP.planet_gtoc5(ast_id)\n\n"
			" - ast_id: a consecutive id from 1 to 7076 (Earth). The order is that of the original"
			"data file distributed by the Russian, see the (`gtoc5 web portal <gtoc5.math.msu.su>`_. Earth is 7076\n\n"
			"Example::\n\n"
			"  russian_ast = planet_gtoc5(1)"
		))
		.def_pickle(generic_pickle_suite<kep_toolbox::asteroid_gtoc5>())
		.def(init<>());

	// A planet from the gtoc2 problem
	class_<kep_toolbox::asteroid_gtoc2,bases<kep_toolbox::planet> >("planet_gtoc2",
		init<const int &>(
			"PyKEP.planet_gtoc2(ast_id)\n\n"
			" - ast_id: Construct from a consecutive id from 0 to 910 (Earth)."
			"The order is that of the original data file from JPL\n\n"
			"    - Group 1:   0 - 95\n"
			"    - Group 2:  96 - 271\n"
			"    - Group 3: 272 - 571\n"
			"    - Group 4: 572 - 909\n"
			"    - Earth:   910\n\n"
			"Example::\n\n"
			"  earth_gtoc2 = planet_gtoc2(910)"
		))
		.def_pickle(generic_pickle_suite<kep_toolbox::asteroid_gtoc2>())
		.def(init<>());

	register_ptr_to_python<kep_toolbox::planet_ptr>();

	// Lambert.
	class_<kep_toolbox::lambert_problem>("lambert_problem","Represents a multiple revolution Lambert's problem",
		init<const kep_toolbox::array3D &, const kep_toolbox::array3D &, const double &, optional<const double &, const int &, const int&> >(
			"lambert_problem(r1, r2, t [, mu = 1, cw = False, multi_revs = 5])\n\n"
			"- r1: starting position (x1,y1,z1)\n"
			"- r2: 3D final position (x2,y2,z2)\n"
			"- t: time of flight\n"
			"- mu: gravitational parameter (default is 1)\n"
			"- cw: True for retrograde motion (clockwise), False if counter-clock wise (default is False)\n"
			"- multi_revs: Maximum number of multirevs to be computed (default is 5)\n"
			"NOTE: Units need to be consistent.\n\n"
			"NOTE: The multirev Lambert's problem will be solved upon construction and its solution stored in data members.\n\n"
			"Example (non-dimensional units used)::\n\n"
			"  l = lambert_problem([1,0,0],[0,1,0],5 * pi / 2. )"
		))
		.def("get_v1",&kep_toolbox::lambert_problem::get_v1,return_value_policy<copy_const_reference>(),
			"Returns a sequence of vectors containing the velocities at r1 of all computed solutions to the Lambert's Problem\n\n"
			"Solutions are stored in order 0 rev, 1rev, 1rev, 2rev, 2rev, ...\n\n"
			"Example (extracts v1 for the 0 revs solution)::\n\n"
			"  v10 = l.get_v1()[0]"
		)
		.def("get_v2",&kep_toolbox::lambert_problem::get_v2,return_value_policy<copy_const_reference>(),
			"Returns a sequence of vectors containing the velocities at r2 of all computed solutions to the Lambert's Problem\n\n"
			"Solutions are stored in order 0 rev, 1rev, 1rev, 2rev, 2rev, ...\n\n"
			"Example (extracts v2 for the 0 revs solution)::\n\n"
			"  v20 = l.get_v2()[0]"
		)
		.def("get_r1",&kep_toolbox::lambert_problem::get_r1,return_value_policy<copy_const_reference>(),
			"Returns a vector containing the r1 defining the Lambert's Problem\n\n"
			"Example ::\n\n"
			"  r1 = l.get_r1()"
		)
		.def("get_r2",&kep_toolbox::lambert_problem::get_r2,return_value_policy<copy_const_reference>(),
			"Returns a vector containing the r2 defining the Lambert's Problem\n\n"
			"Example ::\n\n"
			"  r2 = l.get_r2()"
		)
		.def("get_tof",&kep_toolbox::lambert_problem::get_tof,return_value_policy<copy_const_reference>(),
			"Returns the time of flight defining the Lambert's Problem\n\n"
			"Example::\n\n"
			"  t = l.get_tof()"
		)
		.def("get_mu",&kep_toolbox::lambert_problem::get_mu,return_value_policy<copy_const_reference>(),
			"Returns the gravitational parameter defining the Lambert's Problem\n\n"
			"Example::\n\n"
			"  mu = l.get_mu()"
		)
		.def("get_x",&kep_toolbox::lambert_problem::get_x,return_value_policy<copy_const_reference>(),
			"Returns a sequence containing the x values of all computed solutions to the Lambert's Problem\n\n"
			"Solutions are stored in order 0 rev, 1rev, 1rev, 2rev, 2rev, ...\n\n"
			"Example (extracts x for the 0 revs solution)::\n\n"
			"  x0 = l.get_x()[0]"
		)
		.def("get_iters",&kep_toolbox::lambert_problem::get_iters,return_value_policy<copy_const_reference>(),
			"Returns a sequence containing the number of iterations employed to compute each solution to the Lambert's Problem\n\n"
			"Solutions are stored in order 0 rev, 1rev, 1rev, 2rev, 2rev, ...\n\n"
			"Example (extracts the number of iterations employed for the 0 revs solution)::\n\n"
			"  p0 = l.get_iters()[0]"
		)
		.def("get_Nmax",&kep_toolbox::lambert_problem::get_Nmax,
			"Returns the maximum number of revolutions allowed. The total number of solution to the Lambert's problem will thus be n_sol = Nmax*2 + 1\n\n"
			"Example::\n\n"
			"  Nmax = l.get_Nmax()\n"
			"  n_sol = Nmax*2+1"
		)
		.def(repr(self))
		.def_pickle(generic_pickle_suite<kep_toolbox::lambert_problem>())
		.def(init<>());

	// Lambert problem OLD.
	class_<kep_toolbox::lambert_problemOLD>("lambert_problemOLD","Represents a multiple revolution Lambert's problem",
		init<const kep_toolbox::array3D &, const kep_toolbox::array3D &, const double &, optional<const double &, const int &, const int&> >(
			"lambert_problem(r1, r2, t [, mu = 1, cw = False, multi_revs = 5])\n\n"
			"- r1: starting position (x1,y1,z1)\n"
			"- r2: 3D final position (x2,y2,z2)\n"
			"- t: time of flight\n"
			"- mu: gravitational parameter (default is 1)\n"
			"- cw: True for retrograde motion (clockwise), False if counter-clock wise (default is False)\n"
			"- multi_revs: Maximum number of multirevs to be computed (default is 5)\n"
			"NOTE: Units need to be consistent.\n\n"
			"NOTE: The multirev Lambert's problem will be solved upon construction and its solution stored in data members.\n\n"
			"Example (non-dimensional units used)::\n\n"
			"  l = lambert_problem([1,0,0],[0,1,0],5 * pi / 2. )"
		))
		.def("get_v1",&kep_toolbox::lambert_problemOLD::get_v1,return_value_policy<copy_const_reference>(),
			"Returns a sequence of vectors containing the velocities at r1 of all computed solutions to the Lambert's Problem\n\n"
			"Solutions are stored in order 0 rev, 1rev, 1rev, 2rev, 2rev, ...\n\n"
			"Example (extracts v1 for the 0 revs solution)::\n\n"
			"  v10 = l.get_v1()[0]"
		)
		.def("get_v2",&kep_toolbox::lambert_problemOLD::get_v2,return_value_policy<copy_const_reference>(),
			"Returns a sequence of vectors containing the velocities at r2 of all computed solutions to the Lambert's Problem\n\n"
			"Solutions are stored in order 0 rev, 1rev, 1rev, 2rev, 2rev, ...\n\n"
			"Example (extracts v2 for the 0 revs solution)::\n\n"
			"  v20 = l.get_v2()[0]"
		)
		.def("get_r1",&kep_toolbox::lambert_problemOLD::get_r1,return_value_policy<copy_const_reference>(),
			"Returns a vector containing the r1 defining the Lambert's Problem\n\n"
			"Example ::\n\n"
			"  r1 = l.get_r1()"
		)
		.def("get_r2",&kep_toolbox::lambert_problemOLD::get_r2,return_value_policy<copy_const_reference>(),
			"Returns a vector containing the r2 defining the Lambert's Problem\n\n"
			"Example ::\n\n"
			"  r2 = l.get_r2()"
		)
		.def("get_tof",&kep_toolbox::lambert_problemOLD::get_tof,return_value_policy<copy_const_reference>(),
			"Returns the time of flight defining the Lambert's Problem\n\n"
			"Example::\n\n"
			"  t = l.get_tof()"
		)
		.def("get_mu",&kep_toolbox::lambert_problemOLD::get_mu,return_value_policy<copy_const_reference>(),
			"Returns the gravitational parameter defining the Lambert's Problem\n\n"
			"Example::\n\n"
			"  mu = l.get_mu()"
		)
		.def("get_x",&kep_toolbox::lambert_problemOLD::get_a,return_value_policy<copy_const_reference>(),
			"Returns a sequence containing the a values of all computed solutions to the Lambert's Problem\n\n"
			"Solutions are stored in order 0 rev, 1rev, 1rev, 2rev, 2rev, ...\n\n"
			"Example (extracts a for the 0 revs solution)::\n\n"
			"  x0 = l.get_a()[0]"
		)
		.def("get_iters",&kep_toolbox::lambert_problemOLD::get_iters,return_value_policy<copy_const_reference>(),
			"Returns a sequence containing the number of iterations employed to compute each solution to the Lambert's Problem\n\n"
			"Solutions are stored in order 0 rev, 1rev, 1rev, 2rev, 2rev, ...\n\n"
			"Example (extracts the number of iterations employed for the 0 revs solution)::\n\n"
			"  p0 = l.get_iters()[0]"
		)
		.def("get_Nmax",&kep_toolbox::lambert_problemOLD::get_Nmax,
			"Returns the maximum number of revolutions allowed. The total number of solution to the Lambert's problem will thus be n_sol = Nmax*2 + 1\n\n"
			"Example::\n\n"
			"  Nmax = l.get_Nmax()\n"
			"  n_sol = Nmax*2+1"
		)
		.def(repr(self))
		.def_pickle(generic_pickle_suite<kep_toolbox::lambert_problemOLD>())
		.def(init<>());

	//Lagrangian propagator for keplerian orbits
	def("propagate_lagrangian", &propagate_lagrangian_wrapper,
		"PyKEP.propagate_lagrangian(r,v,t,mu)\n\n"
		"- r: start position, x,y,z\n"
		"- v: start velocity, vx,vy,vz\n"
		"- t: propagation time\n"
		"- mu: central body gravity constant\n\n"
		"Returns a tuple containing r and v, the final position and velocity after the propagation.\n\n"
		"Example::\n\n"
		"  r,v = propagate_lagrangian([1,0,0],[0,1,0],pi/2,1)"
	);


	//Taylor propagation of inertially constant thrust arcs
	def("propagate_taylor",&propagate_taylor_wrapper,
		"PyKEP.propagate_taylor(r,v,m,u,t,mu,veff,log10tol,log10rtol)\n\n"
		"- r: start position, x,y,z\n"
		"- v: start velocity, vx,vy,vz\n"
		"- m: starting mass\n"
		"- t: propagation time\n"
		"- u: fixed inertial thrust, ux,uy,uz\n"
		"- mu: central body gravity constant\n\n"
		"- veff: the product (Isp g0) defining the engine efficiency \n\n"
		"- log10tol: the logarithm of the absolute tolerance passed to taylor propagator \n\n"
		"- log10rtol: the logarithm of the relative tolerance passed to taylor propagator \n\n"
		"Returns a tuple containing r, v, and m the final position, velocity and mass after the propagation.\n\n"
		"Example::\n\n"
		"  r,v,m = propagate_taylor([1,0,0],[0,1,0],100,[0,0,0],pi/2,1,1,-15,-15)"
	);

	//Taylor propagation of inertially constant thrust arcs (using the generalized sundmann variable)
	def("propagate_taylor_s",&propagate_taylor_s_wrapper,
		"PyKEP.propagate_taylor_s(r,v,m,u,s,mu,veff,c=1,alpha,log10tol,log10rtol])\n\n"
		"- r: start position, x,y,z\n"
		"- v: start velocity, vx,vy,vz\n"
		"- m: starting mass\n"
		"- u: fixed inertial thrust, ux,uy,uz\n"
		"- s: propagation pseudo-time\n"
		"- mu: central body gravity constant\n\n"
		"- veff: the product (Isp g0) defining the engine efficiency \n\n"
		"- c: constant coefficient in the generalized Sundmann transform \n\n"
		"- alpha: r exponent in the generalized Sundmann transform \n\n"
		"- log10tol: the logarithm of the absolute tolerance passed to taylor propagator \n\n"
		"- log10rtol: the logarithm of the relative tolerance passed to taylor propagator \n\n"
		"Returns a tuple containing r, v, m and t the final position, velocity, mass and time after the propagation pseudo-time s\n\n"
		"Example::\n\n"
		"  r,v,m,t = propagate_taylor_s([1,0,0],[0,1,0],100,[0,0,0],pi/2,1.0,1.0,1.0,1.0,-10,-10)"
	);

	//Fly-by helper functions
	def("fb_con",&fb_con_wrapper,
		"PyKEP.fb_con(vin,vout,pl)\n\n"
		"- vin: cartesian coordinates of the relative hyperbolic velocity before the fly-by\n"
		"- vout: vout, cartesian coordinates of the relative hyperbolic velocity after the fly-by\n"
		"- pl: fly-by planet\n\n"
		"Returns a tuple containing (eq, ineq). \n"
		"  eq represents the violation of the equality constraint norm(vin)**2 = norm(vout)**2.\n"
		"  ineq represents the violation of the inequality constraint on the hyperbola asymptote maximum deflection\n\n"
		"Example::\n\n"
		"  v2_eq, delta_ineq = fb_con(vin, vout, planet_ss('earth'))\n"
		"  v2_eq = v2_eq / EARTH_VELOCITY**2\n"
	);
	def("fb_prop",&fb_prop_wrapper,
			  "PyKEP.fb_prop(v,v_pla,rp,beta,mu)\n\n"
			  "- v: spacecarft velocity before the encounter (cartesian, absolute)\n"
			  "- v-pla: planet inertial velocity at encounter (cartesian, absolute)\n"
			  "- rp: fly-by radius\n"
			  "- beta: fly-by plane orientation\n"
			  "- mu: planet gravitational constant\n\n"
			  "Returns the cartesian components of the spacecarft velocity after the encounter. \n"
			  "Example::\n\n"
		"  vout = fb_prop([1,0,0],[0,1,0],2,3.1415/2,1)\n"
	);
	def("fb_vel",&fb_vel_wrapper,
		"PyKEP.fb_vel(vin,vout,pl)\n\n"
		"- vin: cartesian coordinates of the relative hyperbolic velocity before the fly-by\n"
		"- vout: vout, cartesian coordinates of the relative hyperbolic velocity after the fly-by\n"
		"- pl: fly-by planet\n\n"
		"Returns a number dV. \n"
        "  dV represents the norm of the delta-V needed to make a given fly-by possible. dV is necessary to fix the magnitude and the direction of vout.\n\n"
		"Example::\n\n"
		"  dV = fb_vel(vin, vout, planet_ss('earth'))\n"
	);

	//Basic Astrodynamics
	def("closest_distance",&closest_distance_wrapper,
			  "PyKEP.closest_distance(r0,v0,r1,v1,mu)\n\n"
			  "- r0: initial position (cartesian)\n"
			  "- v0: initial velocity (cartesian)\n"
			  "- r1: final position (cartesian)\n"
			  "- v1: final velocity (cartesian)\n"
			  "- mu: planet gravitational constant\n\n"
			  "Returns a tuple containing the closest distance along a keplerian orbit defined by r0,v0,r1,v1 (it assumes the orbit actually exists) and the apoapsis radius of the resulting orbit. \n"
			  "Example::\n\n"
		"  d,ra = closest_distance([1,0,0],[0,1,0],[],[],1.0)\n"
	);
}
