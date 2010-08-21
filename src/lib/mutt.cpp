/****************************************************************************
 *  Copyright (C) 2009 Reed A. Cartwright, PhD <reed@scit.us>               *
 ****************************************************************************/

#include <dawg/mutt.h>

using namespace dawg;
 
// random zeta distribution with slope z
// rejection-inversion method of H\"ormann and Derflinger (1996)
// idea borrowed from Indelible
// optimizations for usage in Dawg
boost::uint32_t dawg::mutt::rand_zeta(double z) {
	double z1 = 1.0-z;
	double z2 = 1.0/z1;
	double s = 2.0-zHi(zH(1.5,z1,z2)-pow(2.0,-z),z1,z2);
	double Him = zH(4294967296.5,z1,z2);
	double Hx0 = zH(0.5,z1,z2)-1.0-Him;
	
	double U, X,K;
	for(;;) {
		U = rand_01(); // [0,1)
		U = Him+U*Hx0;
		X = zHi(U,z1,z2);
		K = floor(X+1.5);
		if(K-X < s)
			break;
		if(U > zH(K-0.5,z1,z2)-pow(K,-z))
			break;
	}
	return static_cast<uint32_t>(K);
}

/* New version based on Marsaglia and Tsang, "A Simple Method for
 * generating gamma variables", ACM Transactions on Mathematical
 * Software, Vol 26, No 3 (2000), p363-372.
 *
 * Implemented by J.D.Lamb@btinternet.com, minor modifications for GSL
 * by Brian Gough
 *
 * Imported from GSL's gamma.c.
 */
double dawg::mutt::rand_gamma_high(double a, double b) {
	if(a < 1)
		return rand_gamma(1.0+a,b)*pow(rand_01oo(), 1.0/a);

	double x, v, u;
	double d = a - 1.0/3.0;
	double c = (1.0/3.0)/sqrt(d);

	for(;;) {
		do {
			x = rand_normal(1.0);
			v = 1.0 + c * x;
		} while (v <= 0);
		v = v * v * v;
		u = rand_01oo();
		x *= x;
		if (u < 1 - 0.0331 * x * x) 
			break;
		if (log(u) < 0.5 * x + d * (1 - v + log(v)))
			break;
	}
	return b*d*v;
}

/************************************************************
 * Tables for normal method                                 *
 ************************************************************/

/* tabulated values for the height of the Ziggurat levels */
static const double ytab[128] = {
  1, 0.963598623011, 0.936280813353, 0.913041104253,
  0.892278506696, 0.873239356919, 0.855496407634, 0.838778928349,
  0.822902083699, 0.807732738234, 0.793171045519, 0.779139726505,
  0.765577436082, 0.752434456248, 0.739669787677, 0.727249120285,
  0.715143377413, 0.703327646455, 0.691780377035, 0.68048276891,
  0.669418297233, 0.65857233912, 0.647931876189, 0.637485254896,
  0.62722199145, 0.617132611532, 0.607208517467, 0.597441877296,
  0.587825531465, 0.578352913803, 0.569017984198, 0.559815170911,
  0.550739320877, 0.541785656682, 0.532949739145, 0.524227434628,
  0.515614886373, 0.507108489253, 0.498704867478, 0.490400854812,
  0.482193476986, 0.47407993601, 0.466057596125, 0.458123971214,
  0.450276713467, 0.442513603171, 0.434832539473, 0.427231532022,
  0.419708693379, 0.41226223212, 0.404890446548, 0.397591718955,
  0.390364510382, 0.383207355816, 0.376118859788, 0.369097692334,
  0.362142585282, 0.355252328834, 0.348425768415, 0.341661801776,
  0.334959376311, 0.328317486588, 0.321735172063, 0.31521151497,
  0.308745638367, 0.302336704338, 0.29598391232, 0.289686497571,
  0.283443729739, 0.27725491156, 0.271119377649, 0.265036493387,
  0.259005653912, 0.253026283183, 0.247097833139, 0.241219782932,
  0.235391638239, 0.229612930649, 0.223883217122, 0.218202079518,
  0.212569124201, 0.206983981709, 0.201446306496, 0.195955776745,
  0.190512094256, 0.185114984406, 0.179764196185, 0.174459502324,
  0.169200699492, 0.1639876086, 0.158820075195, 0.153697969964,
  0.148621189348, 0.143589656295, 0.138603321143, 0.133662162669,
  0.128766189309, 0.123915440582, 0.119109988745, 0.114349940703,
  0.10963544023, 0.104966670533, 0.100343857232, 0.0957672718266,
  0.0912372357329, 0.0867541250127, 0.082318375932, 0.0779304915295,
  0.0735910494266, 0.0693007111742, 0.065060233529, 0.0608704821745,
  0.056732448584, 0.05264727098, 0.0486162607163, 0.0446409359769,
  0.0407230655415, 0.0368647267386, 0.0330683839378, 0.0293369977411,
  0.0256741818288, 0.0220844372634, 0.0185735200577, 0.0151490552854,
  0.0118216532614, 0.00860719483079, 0.00553245272614, 0.00265435214565
};

/* tabulated values for 2^24 times x[i]/x[i+1],
 * used to accept for U*x[i+1]<=x[i] without any floating point operations */
static const boost::uint32_t ktab[128] = {
  0, 12590644, 14272653, 14988939,
  15384584, 15635009, 15807561, 15933577,
  16029594, 16105155, 16166147, 16216399,
  16258508, 16294295, 16325078, 16351831,
  16375291, 16396026, 16414479, 16431002,
  16445880, 16459343, 16471578, 16482744,
  16492970, 16502368, 16511031, 16519039,
  16526459, 16533352, 16539769, 16545755,
  16551348, 16556584, 16561493, 16566101,
  16570433, 16574511, 16578353, 16581977,
  16585398, 16588629, 16591685, 16594575,
  16597311, 16599901, 16602354, 16604679,
  16606881, 16608968, 16610945, 16612818,
  16614592, 16616272, 16617861, 16619363,
  16620782, 16622121, 16623383, 16624570,
  16625685, 16626730, 16627708, 16628619,
  16629465, 16630248, 16630969, 16631628,
  16632228, 16632768, 16633248, 16633671,
  16634034, 16634340, 16634586, 16634774,
  16634903, 16634972, 16634980, 16634926,
  16634810, 16634628, 16634381, 16634066,
  16633680, 16633222, 16632688, 16632075,
  16631380, 16630598, 16629726, 16628757,
  16627686, 16626507, 16625212, 16623794,
  16622243, 16620548, 16618698, 16616679,
  16614476, 16612071, 16609444, 16606571,
  16603425, 16599973, 16596178, 16591995,
  16587369, 16582237, 16576520, 16570120,
  16562917, 16554758, 16545450, 16534739,
  16522287, 16507638, 16490152, 16468907,
  16442518, 16408804, 16364095, 16301683,
  16207738, 16047994, 15704248, 15472926
};

/* tabulated values of 2^{-24}*x[i] */
static const double wtab[128] = {
  1.62318314817e-08, 2.16291505214e-08, 2.54246305087e-08, 2.84579525938e-08,
  3.10340022482e-08, 3.33011726243e-08, 3.53439060345e-08, 3.72152672658e-08,
  3.8950989572e-08, 4.05763964764e-08, 4.21101548915e-08, 4.35664624904e-08,
  4.49563968336e-08, 4.62887864029e-08, 4.75707945735e-08, 4.88083237257e-08,
  5.00063025384e-08, 5.11688950428e-08, 5.22996558616e-08, 5.34016475624e-08,
  5.44775307871e-08, 5.55296344581e-08, 5.65600111659e-08, 5.75704813695e-08,
  5.85626690412e-08, 5.95380306862e-08, 6.04978791776e-08, 6.14434034901e-08,
  6.23756851626e-08, 6.32957121259e-08, 6.42043903937e-08, 6.51025540077e-08,
  6.59909735447e-08, 6.68703634341e-08, 6.77413882848e-08, 6.8604668381e-08,
  6.94607844804e-08, 7.03102820203e-08, 7.11536748229e-08, 7.1991448372e-08,
  7.2824062723e-08, 7.36519550992e-08, 7.44755422158e-08, 7.52952223703e-08,
  7.61113773308e-08, 7.69243740467e-08, 7.77345662086e-08, 7.85422956743e-08,
  7.93478937793e-08, 8.01516825471e-08, 8.09539758128e-08, 8.17550802699e-08,
  8.25552964535e-08, 8.33549196661e-08, 8.41542408569e-08, 8.49535474601e-08,
  8.57531242006e-08, 8.65532538723e-08, 8.73542180955e-08, 8.8156298059e-08,
  8.89597752521e-08, 8.97649321908e-08, 9.05720531451e-08, 9.138142487e-08,
  9.21933373471e-08, 9.30080845407e-08, 9.38259651738e-08, 9.46472835298e-08,
  9.54723502847e-08, 9.63014833769e-08, 9.71350089201e-08, 9.79732621669e-08,
  9.88165885297e-08, 9.96653446693e-08, 1.00519899658e-07, 1.0138063623e-07,
  1.02247952126e-07, 1.03122261554e-07, 1.04003996769e-07, 1.04893609795e-07,
  1.05791574313e-07, 1.06698387725e-07, 1.07614573423e-07, 1.08540683296e-07,
  1.09477300508e-07, 1.1042504257e-07, 1.11384564771e-07, 1.12356564007e-07,
  1.13341783071e-07, 1.14341015475e-07, 1.15355110887e-07, 1.16384981291e-07,
  1.17431607977e-07, 1.18496049514e-07, 1.19579450872e-07, 1.20683053909e-07,
  1.21808209468e-07, 1.2295639141e-07, 1.24129212952e-07, 1.25328445797e-07,
  1.26556042658e-07, 1.27814163916e-07, 1.29105209375e-07, 1.30431856341e-07,
  1.31797105598e-07, 1.3320433736e-07, 1.34657379914e-07, 1.36160594606e-07,
  1.37718982103e-07, 1.39338316679e-07, 1.41025317971e-07, 1.42787873535e-07,
  1.44635331499e-07, 1.4657889173e-07, 1.48632138436e-07, 1.50811780719e-07,
  1.53138707402e-07, 1.55639532047e-07, 1.58348931426e-07, 1.61313325908e-07,
  1.64596952856e-07, 1.68292495203e-07, 1.72541128694e-07, 1.77574279496e-07,
  1.83813550477e-07, 1.92166040885e-07, 2.05295471952e-07, 2.22600839893e-07
};

/*
 *     Ziggurat method as implemented in GSL
 *     George Marsaglia, Wai Wan Tsang
 *     The Ziggurat Method for Generating Random Variables
 *     Journal of Statistical Software, vol. 5 (2000), no. 8
 *     http://www.jstatsoft.org/v05/i08/
 */
 /* position of right-most step */
#define PARAM_R 3.44428647676
double dawg::mutt::rand_normal(double sigma) {
	boost::uint32_t i, j;
	int sign;
	double x, y;

	for(;;) {
		boost::uint32_t k = rand_uint32();
		i = (k & 0xFF);
		j = (k >> 8) & 0xFFFFFF;

		sign = (i & 0x80) ? +1 : -1;
		i &= 0x7f;

		x = j * wtab[i];

		if (j < ktab[i])
			break;

		if(i < 127) {
			double y0 = ytab[i], y1 = ytab[i + 1];
			double U1 = rand_01();
			y = y1 + (y0 - y1) * U1;
		} else {
			double U1 = 1.0 - rand_uint32();
			double U2 = rand_01();
			x = PARAM_R - log(U1) / PARAM_R;
			y = exp(-PARAM_R * (x - 0.5 * PARAM_R)) * U2;
		}

		if (y < exp(-0.5 * x * x))
			break;
    }
	return sign * sigma * x;	
}

/************************************************************
 * Tables for exponential                                   *
 ************************************************************/

const double dawg::mutt::ew[256] = {
	1.93114801264183738e-15, 1.41780284879104034e-17, 2.32788249933821363e-17, 3.04878302470640491e-17,
	3.66656977144746503e-17, 4.21793021892895231e-17, 4.72225615568625790e-17, 5.19119154462177003e-17,
	5.63234710839548619e-17, 6.05100826064274740e-17, 6.45101650967273330e-17, 6.83526468037003804e-17,
	7.20599395746889142e-17, 7.56498155373928331e-17, 7.91366439619509290e-17, 8.25322355635187930e-17,
	8.58464361688503647e-17, 8.90875548656472928e-17, 9.22626796296635830e-17, 9.53779145052925832e-17,
	9.84385608745591337e-17, 1.01449258090062829e-16, 1.04414094055853336e-16, 1.07336693234363757e-16,
	1.10220287456701796e-16, 1.13067773464793219e-16, 1.15881760097055209e-16, 1.18664607304178738e-16,
	1.21418458656943468e-16, 1.24145268623263722e-16, 1.26846825606061382e-16, 1.29524771519122670e-16,
	1.32180618515387957e-16, 1.34815763357454272e-16, 1.37431499823676101e-16, 1.40029029468078444e-16,
	1.42609470993212720e-16, 1.45173868448292819e-16, 1.47723198427635692e-16, 1.50258376414474389e-16,
	1.52780262391016343e-16, 1.55289665815956814e-16, 1.57787350054595663e-16, 1.60274036333508941e-16,
	1.62750407280835096e-16, 1.65217110104200616e-16, 1.67674759450782640e-16, 1.70123939987706290e-16,
	1.72565208735682087e-16, 1.74999097184323478e-16, 1.77426113213804898e-16, 1.79846742844306969e-16,
	1.82261451831958036e-16, 1.84670687127635615e-16, 1.87074878212982406e-16, 1.89474438326258843e-16,
	1.91869765589159798e-16, 1.94261244044430252e-16, 1.96649244612990055e-16, 1.99034125978301265e-16,
	2.01416235404858814e-16, 2.03795909496938668e-16, 2.06173474903084218e-16, 2.08549248971237563e-16,
	2.10923540358915133e-16, 2.13296649602382817e-16, 2.15668869648389552e-16, 2.18040486351669971e-16,
	2.20411778941115499e-16, 2.22783020457239353e-16, 2.25154478163313350e-16, 2.27526413932336793e-16,
	2.29899084611801761e-16, 2.32272742368043563e-16, 2.34647635011809059e-16, 2.37024006306533788e-16,
	2.39402096260692933e-16, 2.41782141405477005e-16, 2.44164375058941132e-16, 2.46549027577682940e-16,
	2.48936326597022406e-16, 2.51326497260579653e-16, 2.53719762440079508e-16, 2.56116342946149876e-16,
	2.58516457730823861e-16, 2.60920324082405769e-16, 2.63328157813314467e-16, 2.65740173441476178e-16,
	2.68156584365799837e-16, 2.70577603036235042e-16, 2.73003441118879554e-16, 2.75434309656576242e-16,
	2.77870419225412784e-16, 2.80311980087514308e-16, 2.82759202340497038e-16, 2.85212296063933089e-16,
	2.87671471463158085e-16, 2.90136939010737586e-16, 2.92608909585895194e-16, 2.95087594612190376e-16,
	2.97573206193725213e-16, 3.00065957250147439e-16, 3.02566061650707935e-16, 3.05073734347625209e-16,
	3.07589191508999436e-16, 3.10112650651515528e-16, 3.12644330773167600e-16, 3.15184452486235327e-16,
	3.17733238150736879e-16, 3.20290912008583396e-16, 3.22857700318655782e-16, 3.25433831493026149e-16,
	3.28019536234543689e-16, 3.30615047676007480e-16, 3.33220601521148463e-16, 3.35836436187645822e-16,
	3.38462792952404503e-16, 3.41099916099326015e-16, 3.43748053069806380e-16, 3.46407454616201723e-16,
	3.49078374958506844e-16, 3.51761071944498325e-16, 3.54455807213601347e-16, 3.57162846364746573e-16,
	3.59882459128492793e-16, 3.62614919543700361e-16, 3.65360506139050502e-16, 3.68119502119717615e-16,
	3.70892195559513942e-16, 3.73678879598838586e-16, 3.76479852648778460e-16, 3.79295418601723394e-16,
	3.82125887048875357e-16, 3.84971573505048862e-16, 3.87832799641179983e-16, 3.90709893524981925e-16,
	3.93603189870207624e-16, 3.96513030295003957e-16, 3.99439763589868515e-16, 4.02383745995747033e-16,
	4.05345341492839806e-16, 4.08324922100717903e-16, 4.11322868190383674e-16, 4.14339568808947505e-16,
	4.17375422017632043e-16, 4.20430835243858656e-16, 4.23506225648215281e-16, 4.26602020507155916e-16,
	4.29718657612332713e-16, 4.32856585687521043e-16, 4.36016264824156855e-16, 4.39198166936574195e-16,
	4.42402776238099238e-16, 4.45630589739236208e-16, 4.48882117769261718e-16, 4.52157884522634748e-16,
	4.55458428631724207e-16, 4.58784303767462271e-16, 4.62136079269642762e-16, 4.65514340808707021e-16,
	4.68919691080991670e-16, 4.72352750539554897e-16, 4.75814158162855439e-16, 4.79304572263724803e-16,
	4.82824671341258655e-16, 4.86375154978451191e-16, 4.89956744788614142e-16, 4.93570185413857846e-16,
	4.97216245579170442e-16, 5.00895719205911408e-16, 5.04609426588843403e-16, 5.08358215641162445e-16,
	5.12142963212354152e-16, 5.15964576484106179e-16, 5.19823994449949285e-16, 5.23722189484784844e-16,
	5.27660169010988466e-16, 5.31638977268368925e-16, 5.35659697195905033e-16, 5.39723452433897787e-16,
	5.43831409455963700e-16, 5.47984779841162965e-16, 5.52184822697523327e-16, 5.56432847249287216e-16,
	5.60730215601396693e-16, 5.65078345696050637e-16, 5.69478714477634818e-16, 5.73932861283963536e-16,
	5.78442391483599120e-16, 5.83008980381058637e-16, 5.87634377414005628e-16, 5.92320410669093039e-16,
	5.97068991746008957e-16, 6.01882121002523530e-16, 6.06761893217000486e-16, 6.11710503708971978e-16,
	6.16730254963061902e-16, 6.21823563806853170e-16, 6.26992969199332624e-16, 6.32241140693421147e-16,
	6.37570887643942623e-16, 6.42985169241359472e-16, 6.48487105461890328e-16, 6.54079989036448088e-16,
	6.59767298554456630e-16, 6.65552712834334282e-16, 6.71440126710648815e-16, 6.77433668409100934e-16,
	6.83537718705127301e-16, 6.89756932090684778e-16, 6.96096260207488359e-16, 7.02560977844595785e-16,
	7.09156711844958274e-16, 7.15889473320855210e-16, 7.22765693643812020e-16, 7.29792264752908410e-16,
	7.36976584419124067e-16, 7.44326607216041359e-16, 7.51850902083251109e-16, 7.59558717533774779e-16,
	7.67460055757842639e-16, 7.75565757121578960e-16, 7.83887596862285675e-16, 7.92438396157354901e-16,
	8.01232150211308338e-16, 8.10284176591314635e-16, 8.19611287780612502e-16, 8.29231992858180916e-16,
	8.39166734414679785e-16, 8.49438168364877010e-16, 8.60071496333494140e-16, 8.71094862938790402e-16,
	8.82539833807214079e-16, 8.94441974851986457e-16, 9.06841559713166896e-16, 9.19784440981186494e-16,
	9.33323132942295156e-16, 9.47518170652498405e-16, 9.62439834565847587e-16, 9.78170365478441980e-16,
	9.94806847238387951e-16, 1.01246501442883194e-15, 1.03128436577561659e-15, 1.05143516040445497e-15,
	1.07312819542240413e-15, 1.09662880685174059e-15, 1.12227749093503173e-15, 1.15052129630066614e-15,
	1.18196352833042040e-15, 1.21744628323618153e-15, 1.25819580697551141e-15, 1.30609841071280816e-15,
	1.36427861580578573e-15, 1.43848899321787231e-15, 1.54121907000641935e-15, 1.70910340771680548e-15
};

const double dawg::mutt::ef[256] = {
	1.00000000000000000e+00, 9.38143680862176477e-01, 9.00469929925747703e-01, 8.71704332381204705e-01,
	8.47785500623990496e-01, 8.26993296643051101e-01, 8.08421651523009044e-01, 7.91527636972496285e-01,
	7.75956852040116218e-01, 7.61463388849896838e-01, 7.47868621985195658e-01, 7.35038092431424039e-01,
	7.22867659593572465e-01, 7.11274760805076456e-01, 7.00192655082788606e-01, 6.89566496117078431e-01,
	6.79350572264765806e-01, 6.69506316731925177e-01, 6.60000841079000145e-01, 6.50805833414571433e-01,
	6.41896716427266423e-01, 6.33251994214366398e-01, 6.24852738703666200e-01, 6.16682180915207878e-01,
	6.08725382079622346e-01, 6.00968966365232560e-01, 5.93400901691733762e-01, 5.86010318477268366e-01,
	5.78787358602845359e-01, 5.71723048664826150e-01, 5.64809192912400615e-01, 5.58038282262587892e-01,
	5.51403416540641733e-01, 5.44898237672440056e-01, 5.38516872002862246e-01, 5.32253880263043655e-01,
	5.26104213983620062e-01, 5.20063177368233931e-01, 5.14126393814748894e-01, 5.08289776410643213e-01,
	5.02549501841348056e-01, 4.96901987241549881e-01, 4.91343869594032867e-01, 4.85871987341885248e-01,
	4.80483363930454543e-01, 4.75175193037377708e-01, 4.69944825283960310e-01, 4.64789756250426511e-01,
	4.59707615642138023e-01, 4.54696157474615836e-01, 4.49753251162755330e-01, 4.44876873414548846e-01,
	4.40065100842354173e-01, 4.35316103215636907e-01, 4.30628137288459167e-01, 4.25999541143034677e-01,
	4.21428728997616908e-01, 4.16914186433003209e-01, 4.12454465997161457e-01, 4.08048183152032673e-01,
	4.03694012530530555e-01, 3.99390684475231350e-01, 3.95136981833290435e-01, 3.90931736984797384e-01,
	3.86773829084137932e-01, 3.82662181496010056e-01, 3.78595759409581067e-01, 3.74573567615902381e-01,
	3.70594648435146223e-01, 3.66658079781514379e-01, 3.62762973354817997e-01, 3.58908472948750001e-01,
	3.55093752866787626e-01, 3.51318016437483449e-01, 3.47580494621637148e-01, 3.43880444704502575e-01,
	3.40217149066780189e-01, 3.36589914028677717e-01, 3.32998068761809096e-01, 3.29440964264136438e-01,
	3.25917972393556354e-01, 3.22428484956089223e-01, 3.18971912844957239e-01, 3.15547685227128949e-01,
	3.12155248774179606e-01, 3.08794066934560185e-01, 3.05463619244590256e-01, 3.02163400675693528e-01,
	2.98892921015581847e-01, 2.95651704281261252e-01, 2.92439288161892630e-01, 2.89255223489677693e-01,
	2.86099073737076826e-01, 2.82970414538780746e-01, 2.79868833236972869e-01, 2.76793928448517301e-01,
	2.73745309652802915e-01, 2.70722596799059967e-01, 2.67725419932044739e-01, 2.64753418835062149e-01,
	2.61806242689362922e-01, 2.58883549749016173e-01, 2.55985007030415324e-01, 2.53110290015629402e-01,
	2.50259082368862240e-01, 2.47431075665327543e-01, 2.44625969131892024e-01, 2.41843469398877131e-01,
	2.39083290262449094e-01, 2.36345152457059560e-01, 2.33628783437433291e-01, 2.30933917169627356e-01,
	2.28260293930716618e-01, 2.25607660116683956e-01, 2.22975768058120111e-01, 2.20364375843359439e-01,
	2.17773247148700472e-01, 2.15202151075378628e-01, 2.12650861992978224e-01, 2.10119159388988230e-01,
	2.07606827724221982e-01, 2.05113656293837654e-01, 2.02639439093708962e-01, 2.00183974691911210e-01,
	1.97747066105098818e-01, 1.95328520679563189e-01, 1.92928149976771296e-01, 1.90545769663195363e-01,
	1.88181199404254262e-01, 1.85834262762197083e-01, 1.83504787097767436e-01, 1.81192603475496261e-01,
	1.78897546572478278e-01, 1.76619454590494829e-01, 1.74358169171353411e-01, 1.72113535315319977e-01,
	1.69885401302527550e-01, 1.67673618617250081e-01, 1.65478041874935922e-01, 1.63298528751901734e-01,
	1.61134939917591952e-01, 1.58987138969314129e-01, 1.56854992369365148e-01, 1.54738369384468027e-01,
	1.52637142027442801e-01, 1.50551185001039839e-01, 1.48480375643866735e-01, 1.46424593878344889e-01,
	1.44383722160634720e-01, 1.42357645432472146e-01, 1.40346251074862399e-01, 1.38349428863580176e-01,
	1.36367070926428829e-01, 1.34399071702213602e-01, 1.32445327901387494e-01, 1.30505738468330773e-01,
	1.28580204545228199e-01, 1.26668629437510671e-01, 1.24770918580830933e-01, 1.22886979509545108e-01,
	1.21016721826674792e-01, 1.19160057175327641e-01, 1.17316899211555525e-01, 1.15487163578633506e-01,
	1.13670767882744286e-01, 1.11867631670056283e-01, 1.10077676405185357e-01, 1.08300825451033755e-01,
	1.06537004050001632e-01, 1.04786139306570145e-01, 1.03048160171257702e-01, 1.01322997425953631e-01,
	9.96105836706371317e-02, 9.79108533114922130e-02, 9.62237425504328253e-02, 9.45491893760558727e-02,
	9.28871335560435690e-02, 9.12375166310401969e-02, 8.96002819100328862e-02, 8.79753744672702315e-02,
	8.63627411407569268e-02, 8.47623305323681464e-02, 8.31740930096323966e-02, 8.15979807092374193e-02,
	8.00339475423199054e-02, 7.84819492016064352e-02, 7.69419431704805173e-02, 7.54138887340584096e-02,
	7.38977469923647462e-02, 7.23934808757087517e-02, 7.09010551623718427e-02, 6.94204364987287825e-02,
	6.79515934219366430e-02, 6.64944963853398158e-02, 6.50491177867538045e-02, 6.36154319998073758e-02,
	6.21934154085410362e-02, 6.07830464454796604e-02, 5.93843056334202798e-02, 5.79971756312006592e-02,
	5.66216412837428698e-02, 5.52576896766970305e-02, 5.39053101960460801e-02, 5.25644945930716853e-02,
	5.12352370551262815e-02, 4.99175342827063787e-02, 4.86113855733795036e-02, 4.73167929131815615e-02,
	4.60337610761751836e-02, 4.47622977329432889e-02, 4.35024135688881972e-02, 4.22541224133162543e-02,
	4.10174413804148402e-02, 3.97923910233741393e-02, 3.85789955030748713e-02, 3.73772827729593818e-02,
	3.61872847819314433e-02, 3.50090376973974313e-02, 3.38425821508743577e-02, 3.26879635089595555e-02,
	3.15452321728936225e-02, 3.04144439104666216e-02, 2.92956602246374105e-02, 2.81889487639786461e-02,
	2.70943837809558032e-02, 2.60120466451342208e-02, 2.49420264197317866e-02, 2.38844205115581742e-02,
	2.28393354063852402e-02, 2.18068875042835807e-02, 2.07872040725781138e-02, 1.97804243380097396e-02,
	1.87867007446960235e-02, 1.78062004109113547e-02, 1.68391068260399408e-02, 1.58856218399731561e-02,
	1.49459680116911485e-02, 1.40203914031819428e-02, 1.31091649312549911e-02, 1.22125924262553778e-02,
	1.13310135978346004e-02, 1.04648101810299807e-02, 9.61441364250221163e-03, 8.78031498580897699e-03,
	7.96307743801704347e-03, 7.16335318363499080e-03, 6.38190593731918342e-03, 5.61964220720548909e-03,
	4.87765598354239580e-03, 4.15729512083379705e-03, 3.46026477783690405e-03, 2.78879879357407569e-03,
	2.14596774371890713e-03, 1.53629978030157257e-03, 9.67269282327174319e-04, 4.54134353841496603e-04
};

const boost::uint64_t dawg::mutt::ek[256] = {
	UINT64_C(0xe290a13924be2), UINT64_C(0x0000000000000), UINT64_C(0x9beadebce1892),
	UINT64_C(0xc377ac71f9df8), UINT64_C(0xd4ddb9907584d), UINT64_C(0xde893fb8ca239),
	UINT64_C(0xe4a8e87c43289), UINT64_C(0xe8dff16ae1cb8), UINT64_C(0xebf2deab58c59),
	UINT64_C(0xee49a6e8b9637), UINT64_C(0xf0204efd64ee4), UINT64_C(0xf19bdb8ea3c1a),
	UINT64_C(0xf2d458bbe5bd0), UINT64_C(0xf3da104b78236), UINT64_C(0xf4b86d784571e),
	UINT64_C(0xf577ad8a7784f), UINT64_C(0xf61de83da32ab), UINT64_C(0xf6afb7843cce6),
	UINT64_C(0xf730a57372b44), UINT64_C(0xf7a37651b0e67), UINT64_C(0xf80a5bb6eea51),
	UINT64_C(0xf867189d3cb5a), UINT64_C(0xf8bb1b4f8fbbd), UINT64_C(0xf9079062292b8),
	UINT64_C(0xf94d70ca8d43a), UINT64_C(0xf98d8c7dcaa9a), UINT64_C(0xf9c8928abe083),
	UINT64_C(0xf9ff175b734a6), UINT64_C(0xfa319996bc47d), UINT64_C(0xfa6085f8e9d08),
	UINT64_C(0xfa8c3a62e1991), UINT64_C(0xfab5084e1f660), UINT64_C(0xfadb36c84ccca),
	UINT64_C(0xfaff041086847), UINT64_C(0xfb20a6ea22bb8), UINT64_C(0xfb404fb42cb3d),
	UINT64_C(0xfb5e295158173), UINT64_C(0xfb7a59e99727a), UINT64_C(0xfb95038c8789c),
	UINT64_C(0xfbae44ba684ec), UINT64_C(0xfbc638d822e60), UINT64_C(0xfbdcf89209ffa),
	UINT64_C(0xfbf29a303cfc5), UINT64_C(0xfc0731df1089c), UINT64_C(0xfc1ad1ed6c8b1),
	UINT64_C(0xfc2d8b02b5c89), UINT64_C(0xfc3f6c4d92131), UINT64_C(0xfc5083ac9ba7e),
	UINT64_C(0xfc60ddd1e9cd6), UINT64_C(0xfc7086622e825), UINT64_C(0xfc7f881009f0b),
	UINT64_C(0xfc8decb41ac71), UINT64_C(0xfc9bbd623d7eb), UINT64_C(0xfca9027c5b26d),
	UINT64_C(0xfcb5c3c319c4a), UINT64_C(0xfcc20864b4448), UINT64_C(0xfccdd70a35d40),
	UINT64_C(0xfcd935e34bf80), UINT64_C(0xfce42ab0db8bd), UINT64_C(0xfceebace7ec02),
	UINT64_C(0xfcf8eb3b0d0e7), UINT64_C(0xfd02c0a049b60), UINT64_C(0xfd0c3f59d199d),
	UINT64_C(0xfd156b7b5e27e), UINT64_C(0xfd1e48d670341), UINT64_C(0xfd26daff73551),
	UINT64_C(0xfd2f2552684bf), UINT64_C(0xfd372af7233c1), UINT64_C(0xfd3eeee528f62),
	UINT64_C(0xfd4673e73543b), UINT64_C(0xfd4dbc9e72ff8), UINT64_C(0xfd54cb856dc2c),
	UINT64_C(0xfd5ba2f2c4118), UINT64_C(0xfd62451ba02c2), UINT64_C(0xfd68b415fcff5),
	UINT64_C(0xfd6ef1dabc161), UINT64_C(0xfd75004790eb6), UINT64_C(0xfd7ae120c583f),
	UINT64_C(0xfd809612dbd09), UINT64_C(0xfd8620b40effa), UINT64_C(0xfd8b8285b78fe),
	UINT64_C(0xfd90bcf594b1c), UINT64_C(0xfd95d15efd425), UINT64_C(0xfd9ac10bfa70c),
	UINT64_C(0xfd9f8d364df06), UINT64_C(0xfda437086566b), UINT64_C(0xfda8bf9e3c9ff),
	UINT64_C(0xfdad28062fed5), UINT64_C(0xfdb17141bff2d), UINT64_C(0xfdb59c4648085),
	UINT64_C(0xfdb9a9fda83cc), UINT64_C(0xfdbd9b46e3ed4), UINT64_C(0xfdc170f6b5d05),
	UINT64_C(0xfdc52bd81a3fb), UINT64_C(0xfdc8ccacd07ba), UINT64_C(0xfdcc542dd3902),
	UINT64_C(0xfdcfc30bcb793), UINT64_C(0xfdd319ef77143), UINT64_C(0xfdd6597a0f60b),
	UINT64_C(0xfdd98245a48a2), UINT64_C(0xfddc94e575272), UINT64_C(0xfddf91e64014e),
	UINT64_C(0xfde279ce914cb), UINT64_C(0xfde54d1f0a06a), UINT64_C(0xfde80c52a47d0),
	UINT64_C(0xfdeab7def394e), UINT64_C(0xfded50345eb36), UINT64_C(0xfdefd5be59fa1),
	UINT64_C(0xfdf248e39b26f), UINT64_C(0xfdf4aa064b4b0), UINT64_C(0xfdf6f98435894),
	UINT64_C(0xfdf937b6f30ba), UINT64_C(0xfdfb64f414571), UINT64_C(0xfdfd818d48262),
	UINT64_C(0xfdff8dd07fed9), UINT64_C(0xfe018a08122c4), UINT64_C(0xfe03767adaa5a),
	UINT64_C(0xfe05536c58a14), UINT64_C(0xfe07211ccb4c5), UINT64_C(0xfe08dfc94c532),
	UINT64_C(0xfe0a8fabe8ca1), UINT64_C(0xfe0c30fbb87a6), UINT64_C(0xfe0dc3ecf3a5a),
	UINT64_C(0xfe0f48b107521), UINT64_C(0xfe10bf76a82ef), UINT64_C(0xfe122869e4200),
	UINT64_C(0xfe1383b4327e1), UINT64_C(0xfe14d17c83188), UINT64_C(0xfe1611e74c023),
	UINT64_C(0xfe1745169635a), UINT64_C(0xfe186b2a09177), UINT64_C(0xfe19843ef4e07),
	UINT64_C(0xfe1a90705bf64), UINT64_C(0xfe1b8fd6fb37c), UINT64_C(0xfe1c828951443),
	UINT64_C(0xfe1d689ba4bfd), UINT64_C(0xfe1e4220099a4), UINT64_C(0xfe1f0f26655a0),
	UINT64_C(0xfe1fcfbc726d4), UINT64_C(0xfe2083edc2830), UINT64_C(0xfe212bc3bfeb4),
	UINT64_C(0xfe21c745adfe3), UINT64_C(0xfe225678a8895), UINT64_C(0xfe22d95fa23f4),
	UINT64_C(0xfe234ffb62282), UINT64_C(0xfe23ba4a800d9), UINT64_C(0xfe2418495fddd),
	UINT64_C(0xfe2469f22bffb), UINT64_C(0xfe24af3cce90e), UINT64_C(0xfe24e81ee9858),
	UINT64_C(0xfe25148bcda1a), UINT64_C(0xfe253474703fe), UINT64_C(0xfe2547c75fdc6),
	UINT64_C(0xfe254e70b754f), UINT64_C(0xfe25485a0fd1b), UINT64_C(0xfe25356a71450),
	UINT64_C(0xfe2515864173b), UINT64_C(0xfe24e88f316f1), UINT64_C(0xfe24ae64296fa),
	UINT64_C(0xfe2466e132f60), UINT64_C(0xfe2411df611bd), UINT64_C(0xfe23af34b6f73),
	UINT64_C(0xfe233eb40bf41), UINT64_C(0xfe22c02cee01c), UINT64_C(0xfe22336b81711),
	UINT64_C(0xfe2198385e5cd), UINT64_C(0xfe20ee586b707), UINT64_C(0xfe20358cb5dfb),
	UINT64_C(0xfe1f6d92465b1), UINT64_C(0xfe1e9621f2c9f), UINT64_C(0xfe1daef02c8da),
	UINT64_C(0xfe1cb7accb0a6), UINT64_C(0xfe1bb002d22ca), UINT64_C(0xfe1a9798349b9),
	UINT64_C(0xfe196e0d9140d), UINT64_C(0xfe1832fdebc44), UINT64_C(0xfe16e5fe5f932),
	UINT64_C(0xfe15869dccfd0), UINT64_C(0xfe1414647fe78), UINT64_C(0xfe128ed3cf8b2),
	UINT64_C(0xfe10f565b69cf), UINT64_C(0xfe0f478c633ab), UINT64_C(0xfe0d84b1bdd9e),
	UINT64_C(0xfe0bac36e6687), UINT64_C(0xfe09bd73a6b5c), UINT64_C(0xfe07b7b5d920b),
	UINT64_C(0xfe059a40c26d2), UINT64_C(0xfe03644c5d7f8), UINT64_C(0xfe011504979b2),
	UINT64_C(0xfdfeab887b95d), UINT64_C(0xfdfc26e94a448), UINT64_C(0xfdf986297e306),
	UINT64_C(0xfdf6c83bb8663), UINT64_C(0xfdf3ec0193eee), UINT64_C(0xfdf0f04a5d30a),
	UINT64_C(0xfdedd3d1aa204), UINT64_C(0xfdea953dcfc13), UINT64_C(0xfde7331e3100d),
	UINT64_C(0xfde3abe9626f2), UINT64_C(0xfddffdfb1dbd5), UINT64_C(0xfddc2791ff351),
	UINT64_C(0xfdd826cd068c7), UINT64_C(0xfdd3f9a8d3856), UINT64_C(0xfdcf9dfc95b0d),
	UINT64_C(0xfdcb1176a55fe), UINT64_C(0xfdc65198ba50c), UINT64_C(0xfdc15bb3b2daa),
	UINT64_C(0xfdbc2ce2dc4ae), UINT64_C(0xfdb6c206aaaca), UINT64_C(0xfdb117becb4a1),
	UINT64_C(0xfdab2a6379bf1), UINT64_C(0xfda4f5fdfb4e9), UINT64_C(0xfd9e76401f3a3),
	UINT64_C(0xfd97a67a9ce20), UINT64_C(0xfd9081922142a), UINT64_C(0xfd8901f2d4b02),
	UINT64_C(0xfd812182170e1), UINT64_C(0xfd78d98e23cd3), UINT64_C(0xfd7022bb3f083),
	UINT64_C(0xfd66f4edf96b9), UINT64_C(0xfd5d473200305), UINT64_C(0xfd530f9ccff94),
	UINT64_C(0xfd48432b7b351), UINT64_C(0xfd3cd59a8469e), UINT64_C(0xfd30b9368f909),
	UINT64_C(0xfd23dea45f500), UINT64_C(0xfd16349e2e04a), UINT64_C(0xfd07a7a3ef98b),
	UINT64_C(0xfcf8219b5df05), UINT64_C(0xfce7895bcfcde), UINT64_C(0xfcd5c220ad5e2),
	UINT64_C(0xfcc2aadbc17dc), UINT64_C(0xfcae1d5e81fbd), UINT64_C(0xfc97ed4e778f9),
	UINT64_C(0xfc7fe6d4d720e), UINT64_C(0xfc65ccf39c2fc), UINT64_C(0xfc4957623cb04),
	UINT64_C(0xfc2a2fc826dc8), UINT64_C(0xfc07ee19b01cd), UINT64_C(0xfbe213c1cf493),
	UINT64_C(0xfbb8051ac1567), UINT64_C(0xfb890078d120e), UINT64_C(0xfb5411a5b9a96),
	UINT64_C(0xfb18000547133), UINT64_C(0xfad334827f1e3), UINT64_C(0xfa839276708b9),
	UINT64_C(0xfa263b32e37ed), UINT64_C(0xf9b72d1c52cd2), UINT64_C(0xf930a1a281a04),
	UINT64_C(0xf889f023d820a), UINT64_C(0xf7b577d2be5f3), UINT64_C(0xf69c650c40a8f),
	UINT64_C(0xf51530f0916d9), UINT64_C(0xf2cb0e3c5933e), UINT64_C(0xeeefb15d605d8),
	UINT64_C(0xe6da6ecf27460)
};

