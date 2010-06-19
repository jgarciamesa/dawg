#pragma once
#ifndef DAWG_SUBST_AA_H
#define DAWG_SUBST_AA_H
/****************************************************************************
 *  Copyright (C) 2009-2010 Reed A. Cartwright, PhD <reed@scit.us>          *
 ****************************************************************************/
 
namespace dawg {
 
// name, followed by params, then freqs
template<typename It1, typename It2>
bool subst_model::create_aagtr(const char *mod_name, It1 first1, It1 last1, It2 first2, It2 last2) {
	double d = 0.0;
	int u = 0;
	_model = residue_exchange::AA;
	// do freqs first
	if(!create_freqs(mod_name, first2, last2, &freqs[0], &freqs[20]))
		return false;
	
	// fill params array
	double params[190];
	u = 0;
	for(;first1 != last1 && u<190;++first1,++u) {
		if(*first1 < 0)
			return DAWG_ERROR("Invalid subst model; aagtr parameter #" << u
				<< " '" << *first1 << "' is not >= 0.");
		params[u] = *first1;
	}
	if(u != 190)
		return DAWG_ERROR("Invalid subst model; aagtr requires 190 parameters.");
	
	// construct substitution matrix
	// do this locally to enable possible optimizations?
	double s[20][20];
	double rs[20];
	u = 0;
	double aa = 0.0;
	for(int i=0;i<20;++i) {
		s[i][i] = 0.0;
		for(int j=i+1;j<20;++j) {
			s[i][j] = s[j][i] = params[u++];
			aa = std::max(aa,s[i][j]);
		}
	}
	// scale the matrix to substitution time and uniformize
	d = 0.0;
	uni_scale = 0.0;
	for(int i=0;i<20;++i) {
		for(int j=0;j<20;++j) {
			s[i][j] *= freqs[j];
			d += s[i][j]*freqs[i];
		}
	}
	for(int i=0;i<20;++i) {
		rs[i] = 0.0;
		for(int j=0;j<20;++j) {
			s[i][j] /= d;
			rs[i] += s[i][j];
		}
		uni_scale = std::max(uni_scale, rs[i]);
	}
	// create pseudosubstitutions and transition frequencies
	for(int i=0;i<20;++i)
		s[i][i] = uni_scale - rs[i];
	for(int i=0;i<20;++i) {
		for(int j=0;j<20;++j)
			s[i][j] /= uni_scale;
	}
	
	// create cumulative frequencies
	d = 0.0;
	for(int i=0;i<19;++i) {
		d += freqs[i];
		freqs[i] = d;
	}
	// we will include 32 sites in our binary search
	// so fill them with 1.0
	std::fill(&freqs[19],&freqs[32], 1.0);
	for(int i=0;i<20;++i) {
		d = 0.0;
		for(int j=0;j<19;++j) {
			d += s[i][j];
			table[i][j] = d;
		}
		// we will include 32 sites in our binary search
		// so fill them with 1.0
		std::fill(&table[i][19],&table[i][32], 1.0);
	}
	name = mod_name;
	do_op_f = &subst_model::do_aagtr_f;
	do_op_s = &subst_model::do_aagtr_s;
	
	return true;
}

template<typename It1, typename It2>
bool subst_model::create_equ(const char *, It1 first1, It1 last1, It2 first2, It2 last2) {
	static const double s[190] = {
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
	};
	static const double p[20] = {
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
	};
	if(first2 != last2) { //+F model
		return create_aagtr("equ+f", &s[0], &s[190], first2, last2);
	}
	return create_aagtr("equ", &s[0], &s[190], &p[0], &p[20]);
}

template<typename It1, typename It2>
bool subst_model::create_wag(const char *, It1 first1, It1 last1, It2 first2, It2 last2) {
	static const double s[190] = {
		1.0270400, 0.7389980, 1.5828500, 0.2104940, 1.4167200, 0.3169540, 0.1933350, 0.9062650, 0.3979150, 0.8934960,
		0.5098480, 1.4385500, 0.9085980, 0.5515710, 3.3707900, 2.1211100, 2.0060100, 0.1131330, 0.2407350, 0.0302949,
		0.0213520, 0.3980200, 0.3066740, 0.2489720, 0.1701350, 0.0740339, 0.3842870, 0.3904820, 0.2652560, 0.1094040,
		0.0988179, 0.5281910, 1.4076600, 0.5129840, 1.0021400, 0.7170700, 0.5438330, 6.1741600, 0.0467304, 0.8655840,
		0.9306760, 0.0394370, 0.4798550, 0.0848047, 0.1037540, 5.4294200, 0.4239840, 0.6167830, 0.1473040, 1.0717600,
		0.3748660, 0.1523350, 0.1297670, 0.3257110, 0.0811339, 0.5677170, 0.5700250, 0.1273950, 2.5844300, 0.1542630,
		0.3151240, 0.9471980, 0.6823550, 5.4694700, 0.4391570, 0.7049390, 0.8227650, 0.5887310, 0.1565570, 0.1963030,
		0.0499310, 0.6793710, 1.0594700, 0.0888360, 2.1151700, 1.1906300, 0.0961621, 0.1614440, 0.0999208, 0.1027110,
		0.5459310, 0.1719030, 0.6498920, 1.5296400, 6.4542800, 0.2494100, 0.0304501, 0.3735580, 0.0613037, 0.1741000,
		1.1255600, 0.2435700, 0.3300520, 0.5846650, 1.3418200, 0.2258330, 0.1872470, 0.3369830, 0.1036040, 0.1381900,
		0.8904320, 0.4994620, 0.4041410, 3.9562900, 0.6961980, 4.2941100, 2.1371500, 0.7401690, 0.4733070, 0.1183580,
		0.2625690, 3.8734400, 0.3238320, 3.1709700, 4.2574600, 0.5542360, 0.0999288, 0.1139170, 0.1869790, 0.3194400,
		1.4581600, 7.8213000, 0.2124830, 0.4201700, 0.2575550, 0.9342760, 3.0120100, 0.5568960, 3.8949000, 5.3514200,
		0.9671300, 1.3869800, 0.3054340, 0.1375050, 0.1332640, 4.8540200, 0.1315280, 0.4158440, 0.8694890, 0.4976710,
		0.3447390, 0.3266220, 1.8003400, 0.6653090, 0.3986180, 0.1982210, 0.1713290, 1.5452600, 0.6831620, 0.4939050,
		1.5161200, 2.0584500, 0.5157060, 0.4284370, 0.1950810, 1.5436400, 0.6353460, 3.9742300, 2.0300600, 0.1962460,
		0.0719167, 1.0860000, 0.9333720, 0.6794890, 1.6132800, 0.7953840, 0.3148870, 0.1394050, 0.2160460, 3.0355000,
		1.0288700, 0.8579280, 0.3012810, 0.2157370, 0.2277100, 1.2241900, 0.5544130, 0.2518490, 1.1639200, 0.3815330,
		4.3780200, 0.2327390, 0.5237420, 0.7869930, 1.3882300, 0.1108640, 0.2911480, 0.3653690, 0.3147300, 2.4853900
	};
	static const double p[20] = {
		0.0866279, 0.0193078, 0.0570451, 0.0580589, 0.0384319, 0.0832518, 0.0244313, 0.0484660, 0.0620286, 0.0862090,
		0.0195027, 0.0390894, 0.0457631, 0.0367281, 0.0439720, 0.0695179, 0.0610127, 0.0708956, 0.0143859, 0.0352742
	};
	if(first2 != last2) { //+F model
		return create_aagtr("wag+f", &s[0], &s[190], first2, last2);
	}
	return create_aagtr("wag", &s[0], &s[190], &p[0], &p[20]);
}

template<typename It1, typename It2>
bool subst_model::create_wagstar(const char *, It1 first1, It1 last1, It2 first2, It2 last2) {
	static const double s[190] = {
		1.2132400, 0.7311520, 1.5578800, 0.2131790, 1.4199300, 0.3176840, 0.2145960, 0.8816390, 0.4008220, 0.8874580,
		0.5143470, 1.5186100, 1.0334400, 0.5897180, 3.5249900, 2.2416100, 1.9249600, 0.1353950, 0.2703210, 0.0379056,
		0.0284956, 0.4850010, 0.3125440, 0.3414790, 0.1989580, 0.0719929, 0.4511240, 0.4286480, 0.2335270, 0.1090810,
		0.0999068, 0.5684490, 1.3522100, 0.5229570, 1.1089900, 0.7280650, 0.4819540, 6.0429900, 0.0458258, 0.8835700,
		0.9585290, 0.0390513, 0.4803080, 0.0869637, 0.0992829, 5.3082100, 0.4441520, 0.6573640, 0.1590540, 1.0996500,
		0.3951760, 0.1554190, 0.1421590, 0.3261910, 0.0873936, 0.5886090, 0.5991880, 0.1245530, 2.4539200, 0.1549360,
		0.2944810, 1.0012200, 0.7205670, 5.6037000, 0.4436850, 0.8220250, 0.8897650, 0.5884430, 0.1763970, 0.2096210,
		0.0552962, 0.6317130, 1.0645800, 0.0832422, 2.1041400, 1.1451600, 0.0848492, 0.1652050, 0.1092410, 0.1227920,
		0.5639990, 0.1882370, 0.6530150, 1.5868100, 6.4926900, 0.2795420, 0.0310522, 0.3815140, 0.0674430, 0.1845450,
		1.1271700, 0.2546260, 0.3468230, 0.6297680, 1.3361800, 0.2364890, 0.1900950, 0.3664670, 0.1089820, 0.1629750,
		0.8544850, 0.5089520, 0.4011700, 3.9337000, 0.7221230, 4.8736600, 2.3121100, 0.8766880, 0.5499200, 0.1197490,
		0.2612230, 4.3177200, 0.3205970, 3.1554000, 3.9464600, 0.5273210, 0.1117220, 0.1259990, 0.1872620, 0.3217740,
		1.4887600, 7.4837600, 0.2595840, 0.4400900, 0.2550920, 0.8770570, 2.8810200, 0.5882030, 4.1912500, 5.7411900,
		1.0531400, 1.4517300, 0.3003430, 0.1592610, 0.1556230, 4.8195600, 0.1443540, 0.4228510, 0.8732660, 0.5182100,
		0.3519130, 0.3515640, 1.8210500, 0.7060820, 0.4277180, 0.1984040, 0.1798580, 1.6401800, 0.6608160, 0.5540770,
		1.5687300, 2.0332400, 0.5652990, 0.4370690, 0.2049050, 1.6229900, 0.6741600, 3.9012700, 2.0678700, 0.1933230,
		0.0746093, 1.0526900, 0.9131790, 0.7114980, 1.5469400, 0.8025310, 0.3257450, 0.1350240, 0.2129450, 3.0280800,
		0.8790800, 0.8293150, 0.3289300, 0.2081630, 0.2104940, 1.3561100, 0.5941770, 0.2828920, 1.2408600, 0.3867140,
		4.0250700, 0.2376900, 0.5282490, 0.7421540, 1.4088000, 0.1185840, 0.2864430, 0.3968840, 0.3533580, 2.4226100
	};
	static const double p[20] = {
		0.0866279, 0.0193078, 0.0570451, 0.0580589, 0.0384319, 0.0832518, 0.0244313, 0.0484660, 0.0620286, 0.0862090,
		0.0195027, 0.0390894, 0.0457631, 0.0367281, 0.0439720, 0.0695179, 0.0610127, 0.0708956, 0.0143859, 0.0352742
	};
	if(first2 != last2) { //+F model
		return create_aagtr("wagstar+f", &s[0], &s[190], first2, last2);
	}
	return create_aagtr("wagstar", &s[0], &s[190], &p[0], &p[20]);
}

template<typename It1, typename It2>
bool subst_model::create_lg(const char *, It1 first1, It1 last1, It2 first2, It2 last2) {
	static const double s[190] = {
		2.4890840, 0.3951440, 1.0385450, 0.2537010, 2.0660400, 0.3588580, 0.1498300, 0.5365180, 0.3953370, 1.1240350,
		0.2768180, 1.1776510, 0.9698940, 0.4250930, 4.7271820, 2.1395010, 2.5478700, 0.1807170, 0.2189590, 0.0625560,
		0.0034990, 1.1052510, 0.5692650, 0.6405430, 0.3206270, 0.0132660, 0.5940070, 0.8936800, 0.5287680, 0.0753820,
		0.0848080, 0.5345510, 2.7844780, 1.1434800, 1.9592910, 0.6701280, 1.1655320, 5.2438700, 0.0174160, 0.8449260,
		0.9271140, 0.0106900, 0.2829590, 0.0150760, 0.0255480, 5.0761490, 0.3944560, 0.5233860, 0.1239540, 1.2402750,
		0.4258600, 0.0379670, 0.0298900, 0.1351070, 0.0188110, 0.3488470, 0.4238810, 0.0442650, 1.8071770, 0.0696730,
		0.1737350, 0.5417120, 0.4194090, 4.1285910, 0.3639700, 0.6119730, 0.6045450, 0.2450340, 0.0778520, 0.1200370,
		0.0895860, 0.6821390, 1.1127270, 0.0239180, 2.5926920, 1.7988530, 0.0895250, 0.0944640, 0.0358550, 0.0527220,
		0.3618190, 0.1650010, 0.6546830, 2.4571210, 7.8039020, 0.3114840, 0.0087050, 0.2966360, 0.0442610, 0.1395380,
		1.4376450, 0.1969610, 0.2679590, 0.3901920, 1.7399900, 0.1298360, 0.0767010, 0.2684910, 0.0546790, 0.1088820,
		0.6972640, 0.3663170, 0.4424720, 4.5092380, 0.5088510, 4.8135050, 2.4266010, 0.9900120, 0.5842620, 0.1190130,
		0.5970540, 5.3068340, 0.1590690, 4.1450670, 4.2736070, 0.1915030, 0.0782810, 0.0728540, 0.1269910, 0.0641050,
		1.0337390,10.6491070, 0.1116600, 0.2325230, 0.1375000, 0.6566040, 2.1450780, 0.3903220, 3.2342940, 6.3260670,
		0.7486830, 1.1368630, 0.1852020, 0.0499060, 0.1319320, 6.3123580, 0.0684270, 0.2490600, 0.5824570, 0.3018480,
		0.1822870, 0.3029360, 1.7027450, 0.6196320, 0.2996480, 0.3710040, 0.0998490, 1.6725690, 0.4841330, 0.3469600,
		2.0203660, 1.8987180, 0.6961750, 0.4813060, 0.1617870, 1.6957520, 0.7518780, 4.0083580, 2.0006790, 0.0836880,
		0.0453760, 0.6120250, 0.6242940, 0.3325330, 1.3381320, 0.5714680, 0.2965010, 0.0951310, 0.0896130, 2.8079080,
		1.2238280, 1.0801360, 0.2103320, 0.2361990, 0.2573360, 0.8581510, 0.5789870, 0.1708870, 0.5936070, 0.3144400,
		6.4722790, 0.0983690, 0.2488620, 0.4005470, 2.1881580, 0.1408250, 0.2458410, 0.1895100, 0.2493130, 3.1518150
	};
	static const double p[20] = {
		0.0790660, 0.0129370, 0.0530520, 0.0715860, 0.0423020, 0.0573370, 0.0223550, 0.0621570, 0.0646000, 0.0990810,
		0.0229510, 0.0419770, 0.0440400, 0.0407670, 0.0559410, 0.0611970, 0.0532870, 0.0691470, 0.0120660, 0.0341550
	};
	if(first2 != last2) { //+F model
		return create_aagtr("lg+f", &s[0], &s[190], first2, last2);
	}
	return create_aagtr("lg", &s[0], &s[190], &p[0], &p[20]);
}

template<typename It1, typename It2>
bool subst_model::create_jtt(const char *, It1 first1, It1 last1, It2 first2, It2 last2) {
	static const double s[190] = {
		0.5744780, 0.8274450, 1.0666810, 0.1382930, 1.7401590, 0.2199700, 0.3616840, 0.3694370, 0.3100070, 0.4693950,
		0.5579670, 1.9595990, 0.5567250, 0.5316780, 3.8870950, 4.5825650, 2.9241610, 0.0843290, 0.1394920, 0.1056250,
		0.0539070, 0.6783350, 0.5463890, 0.7249980, 0.1505590, 0.0490090, 0.1645930, 0.4092020, 0.3133110, 0.1236530,
		0.0913040, 1.0198430, 2.1553310, 0.4698230, 0.6213230, 1.1041810, 2.1148520, 7.7665570, 0.0325220, 1.2724340,
		1.0323420, 0.1159680, 0.2824660, 0.0614860, 0.1900010, 5.5495300, 0.1271640, 0.5216460, 0.1548990, 0.5892680,
		0.4251590, 0.3152610, 0.0574660, 0.4539520, 0.0438290, 1.1156320, 0.2437680, 0.1117730, 1.7316840, 0.0974850,
		0.1750840, 0.5781150, 0.1919940, 3.4177060, 0.3184830, 0.3124490, 0.3315840, 0.4652710, 0.1143810, 0.0634520,
		0.0502120, 0.4534280, 0.7770900, 0.0245210, 2.5002940, 0.4361810, 0.0734810, 0.1484830, 0.0456830, 0.0653140,
		0.9439710, 0.1389040, 0.5934780, 0.5379220, 5.4842360, 0.2016960, 0.0537690, 0.2698400, 0.0694920, 0.1303790,
		0.7733130, 0.2080810, 0.2312940, 1.3596520, 1.8742960, 0.3168620, 0.4701400, 0.5441800, 0.0525000, 0.1817880,
		0.5250960, 0.5405710, 0.3296600, 4.0257780, 1.1419610, 5.6840800, 3.2106710, 0.7434580, 0.4773550, 0.1218270,
		0.1281930, 5.8484000, 0.2025620, 2.3351390, 4.8316660, 0.4910030, 0.0985800, 0.0782700, 0.2391950, 0.4051190,
		2.5538060, 9.5339430, 0.1345100, 0.3034450, 0.1464810, 0.6245810, 2.5295170, 0.2163450, 2.9667320, 6.5292550,
		0.4744780, 0.9656410, 0.1240660, 0.0891340, 0.0879040, 3.8569060, 0.1372890, 1.0605040, 0.7090040, 0.3722610,
		0.5925110, 0.2725140, 1.7614390, 0.5303240, 0.2410940, 0.3307200, 0.1642150, 0.4569010, 0.4310450, 0.2855640,
		2.1147280, 3.0385330, 0.2013340, 0.1898700, 0.1218040, 0.7688340, 0.4510950, 5.0579640, 2.3513110, 0.1645250,
		0.0277000, 0.7006930, 1.6081260, 0.7104890, 2.7884060, 1.1769610, 0.2115610, 0.0699650, 0.1138500, 3.0219950,
		0.5488070, 0.5238250, 0.1797710, 0.1722060, 0.2547450, 1.0015510, 0.6502820, 0.1719950, 1.2579610, 0.2356010,
		4.7776470, 0.4085320, 0.3109270, 0.6286080, 1.1439800, 0.0805560, 0.2010940, 0.2396970, 0.1654730, 0.7478890
	}; 
	static const double p[20] = {
		0.0768620, 0.0202790, 0.0512690, 0.0618200, 0.0405300, 0.0747140, 0.0229830, 0.0525690, 0.0594980, 0.0911110,
		0.0234140, 0.0425460, 0.0505320, 0.0410610, 0.0510570, 0.0682250, 0.0585180, 0.0663740, 0.0143360, 0.0323030
	};
	if(first2 != last2) { //+F model
		return create_aagtr("jtt+f", &s[0], &s[190], first2, last2);
	}
	return create_aagtr("jtt", &s[0], &s[190], &p[0], &p[20]);
}

template<typename It1, typename It2>
bool subst_model::create_dayhoff(const char *, It1 first1, It1 last1, It2 first2, It2 last2) {
	static const double s[190] = {
		0.3600160, 1.1998050, 1.9611670, 0.1836410, 2.3861110, 0.2281160, 0.6534160, 0.2586350, 0.4064310, 0.7178400,
		0.9844740, 2.4859200, 0.8877530, 0.2678280, 4.0518700, 3.6803650, 2.0595640, 0.0000000, 0.2441390, 0.0000000,
		0.0000000, 0.0000000, 0.1072780, 0.2827290, 0.4380740, 0.0000000, 0.0000000, 0.0000000, 0.0000000, 0.1875500,
		0.0000000, 0.2323740, 1.5983560, 0.1623660, 0.4846780, 0.0000000, 0.9531640,11.3886590, 0.0000000, 1.2409810,
		0.8682410, 0.2392480, 0.7169130, 0.0000000, 0.0000000, 8.9315150, 0.1339400, 1.3485510, 0.0000000, 0.9560970,
		0.6609300, 0.1783160, 0.0000000, 0.0000000, 0.0000000, 0.8119070, 0.4394690, 0.6095260, 0.8300780, 0.1128800,
		0.3048030, 1.4934090, 0.5070030, 7.0860220, 0.0000000, 0.7939990, 0.3401560, 0.3672500, 0.0000000, 0.2147170,
		0.1534780, 0.4759270, 1.9519510, 0.0000000, 1.5651600, 0.9218600, 0.1385030, 0.1105060, 0.0000000, 0.1369060,
		0.4599010, 0.1366550, 0.1236060, 0.7623540, 6.9526290, 0.1068020, 0.0000000, 0.2676830, 0.0715140, 0.1703720,
		1.3853520, 0.3471530, 0.2815810, 0.0877910, 2.3222430, 0.3066620, 0.5381650, 0.0000000, 0.0000000, 0.0769810,
		0.2704750, 0.4435040, 0.0000000, 5.2900240, 0.9337090, 6.0116130, 2.3831480, 0.3536430, 0.2263330, 0.4387150,
		0.2705640, 1.2654000, 0.4608570, 2.5566850, 3.3327320, 0.7680240, 0.1191520, 0.1803930, 0.6326290, 0.2479550,
		1.9007390, 8.8100380, 0.0000000, 0.3748340, 0.1806290, 2.4117390, 3.1483710, 0.3354190, 1.5190780, 4.6101240,
		0.9545570, 1.3505990, 0.1038500, 0.0000000, 0.1321420, 5.2301150, 0.3411130, 0.3162580, 0.7307720, 0.1549240,
		0.1714320, 0.3310900, 1.7451560, 0.4617760, 0.2865720, 0.0000000, 0.1702050, 1.1274990, 0.8963210, 0.6199510,
		1.0315340, 2.5659550, 0.0000000, 0.0000000, 0.4192440, 1.0285090, 0.3270590, 4.8858920, 2.2716970, 0.1580670,
		0.2249680, 0.9469400, 1.5261880, 1.0283130, 2.4272020, 0.7828570, 0.4850260, 0.0000000, 0.0000000, 2.4399390,
		0.5618280, 0.5256510, 0.3469830, 0.0000000, 0.0000000, 1.5315900, 0.2657450, 0.2403680, 2.0013750, 0.0780120,
		5.4366740, 0.3038360, 0.7408190, 0.3362890, 1.5619970, 0.0000000, 0.4178390, 0.0000000, 0.2793790, 0.6080700
	};
	static const double p[20] = {
		0.0871270, 0.0334740, 0.0468720, 0.0495300, 0.0397720, 0.0886120, 0.0336190, 0.0368860, 0.0804810, 0.0853570,
		0.0147530, 0.0404320, 0.0506800, 0.0382550, 0.0409040, 0.0695770, 0.0585420, 0.0647180, 0.0104940, 0.0299160
	};
	if(first2 != last2) { //+F model
		return create_aagtr("dayhoff+f", &s[0], &s[190], first2, last2);
	}
	return create_aagtr("dayhoff", &s[0], &s[190], &p[0], &p[20]);
}

template<typename It1, typename It2>
bool subst_model::create_molphy(const char *, It1 first1, It1 last1, It2 first2, It2 last2) {
	static const double s[190] = {
		0.3652550, 1.1967940, 1.9431560, 0.1826270, 2.3762090, 0.2255960, 0.6515340, 0.2602440, 0.4082230, 0.7061620,
		0.9953190, 2.4708390, 0.8939100, 0.2672570, 4.0282420, 3.7155250, 2.0510450, 0.0106730, 0.2435010, 0.0070720,
		0.0040830, 0.0240740, 0.1081970, 0.2832220, 0.4424350, 0.0022340, 0.0108240, 0.0392110, 0.0204640, 0.1888140,
		0.0060670, 0.2348720, 1.6094870, 0.1660310, 0.4888880, 0.0962950, 0.9629460,11.2369280, 0.0035970, 1.2306670,
		0.8550750, 0.2375580, 0.7183660, 0.0071800, 0.0323180, 8.9922460, 0.1325710, 1.3522180, 0.0164930, 0.9465520,
		0.6644530, 0.1768290, 0.0089790, 0.0225410, 0.0054910, 0.7997780, 0.4299310, 0.6011840, 0.8262040, 0.1121600,
		0.2965820, 1.4935250, 0.4984680, 7.0578700, 0.0155010, 0.7808140, 0.3396840, 0.3617760, 0.0202610, 0.2118510,
		0.1517450, 0.4673110, 1.9323900, 0.0018430, 1.5608150, 0.9003460, 0.1390250, 0.1090430, 0.0050010, 0.1356340,
		0.4539520, 0.1369750, 0.1222150, 0.7944280, 6.8850350, 0.1050150, 0.0064050, 0.2677850, 0.0714090, 0.1666190,
		1.3924740, 0.3430430, 0.2818800, 0.0870850, 2.2953030, 0.3077960, 0.5328210, 0.0357050, 0.0047420, 0.0757790,
		0.2687040, 0.4398140, 0.0347490, 5.2806130, 0.9163030, 5.9766610, 2.3479590, 0.3471370, 0.2255990, 0.4313650,
		0.2803860, 1.2461680, 0.4616270, 2.5563200, 3.2635920, 0.7729630, 0.1178840, 0.1808120, 0.6284280, 0.2453880,
		1.9101800, 8.7337500, 0.0273800, 0.3721700, 0.1822510, 2.3832840, 3.1975740, 0.3349070, 1.5365920, 4.6212490,
		0.9533140, 1.3697050, 0.1038870, 0.0094930, 0.1324070, 5.1591020, 0.3458150, 0.3152060, 0.7378650, 0.1550100,
		0.1709010, 0.3351770, 1.7427070, 0.4860020, 0.2866140, 0.0529570, 0.1661720, 1.1149930, 0.8783980, 0.6052870,
		1.0227090, 2.5095730, 0.0449960, 0.0318620, 0.4205860, 1.0452930, 0.3293970, 4.9027510, 2.3148050, 0.1588860,
		0.2383280, 0.9533140, 1.5249010, 1.0182020, 2.3944180, 0.7842240, 0.4793000, 0.0099320, 0.0121380, 2.4475380,
		0.5614690, 0.5334710, 0.3473580, 0.0266590, 0.0278800, 1.5168860, 0.2672540, 0.2384840, 2.0926410, 0.0775050,
		5.4475140, 0.3003120, 0.7717310, 0.3329070, 1.5677870, 0.0116370, 0.4200390, 0.0350820, 0.2770330, 0.6355410
	};
	static double p[20] = {
		0.0870000, 0.0330000, 0.0470000, 0.0500000, 0.0400000, 0.0890000, 0.0340000, 0.0370000, 0.0800000, 0.0850000,
		0.0150000, 0.0400000, 0.0510000, 0.0380000, 0.0410000, 0.0700000, 0.0580000, 0.0650000, 0.0100000, 0.0300000
	};
	if(first2 != last2) { //+F model
		return create_aagtr("molphy+f", &s[0], &s[190], first2, last2);
	}
	return create_aagtr("molphy", &s[0], &s[190], &p[0], &p[20]);
}

} // namespace dawg
 
#endif // DAWG_SUBST_AA_H
 
