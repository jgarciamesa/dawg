#pragma once
#ifndef DAWG_SUBST_DNA_H
#define DAWG_SUBST_DNA_H
/****************************************************************************
 *  Copyright (C) 2009-2010 Reed A. Cartwright, PhD <reed@scit.us>          *
 ****************************************************************************/

namespace dawg {

// name, followed by params, then freqs
template<typename It1, typename It2>
bool subst_model::create_gtr(const char *mod_name, It1 first1, It1 last1, It2 first2, It2 last2) {
	double d = 0.0;
	int u = 0;
	_model = residue_exchange::DNA;
	// do freqs first
	if(!create_freqs(mod_name, first2, last2, &freqs[0], &freqs[4]))
		return false;
	
	// fill params array
	double params[6];
	u = 0;
	for(;first1 != last1 && u<6;++first1,++u) {
		if(*first1 < 0)
			return DAWG_ERROR("Invalid subst model; gtr parameter #" << u
				<< " '" << *first1 << "' is not >= 0.");
		params[u] = *first1;
	}
	if(u != 6)
		return DAWG_ERROR("Invalid subst model; gtr requires six parameters.");
	
	// construct substitution matrix
	// do this locally to enable possible optimizations?
	double s[4][4];
	double rs[4];
	s[0][0] = s[1][1] = s[2][2] = s[3][3] = 0.0;
	s[0][1] = s[1][0] = params[0]; // A-C
	s[0][2] = s[2][0] = params[1]; // A-G
	s[0][3] = s[3][0] = params[2]; // A-T
	s[1][2] = s[2][1] = params[3]; // C-G
	s[1][3] = s[3][1] = params[4]; // C-T
	s[2][3] = s[3][2] = params[5]; // G-T
	// scale the matrix to substitution time and uniformize
	d = 0.0;
	uni_scale = 0.0;
	for(int i=0;i<4;++i) {
		for(int j=0;j<4;++j) {
			s[i][j] *= freqs[j];
			d += s[i][j]*freqs[i];
		}
	}
	for(int i=0;i<4;++i) {
		rs[i] = 0.0;
		for(int j=0;j<4;++j) {
			s[i][j] /= d;
			rs[i] += s[i][j];
		}
		uni_scale = std::max(uni_scale, rs[i]);
	}
	// create pseudosubstitutions and transition frequencies
	for(int i=0;i<4;++i)
		s[i][i] = uni_scale - rs[i];
	for(int i=0;i<4;++i) {
		for(int j=0;j<4;++j)
			s[i][j] /= uni_scale;
	}
	
	// create cumulative frequencies
	d = 0.0;
	for(int i=0;i<3;++i) {
		d += freqs[i];
		freqs[i] = d;
	}
	freqs[3] = 1.0;
	for(int i=0;i<4;++i) {
		d = 0.0;
		for(int j=0;j<3;++j) {
			d += s[i][j];
			table[i][j] = d;
		}
		table[i][3] = 1.0;
	}
	name = mod_name;
	do_op_f = &subst_model::do_gtr_f;
	do_op_s = &subst_model::do_gtr_s;
	
	return true;
}
	
template<typename It1, typename It2>
bool subst_model::create_jc(const char *, It1 first1, It1 last1, It2 first2, It2 last2) {
	// equal rates and frequencies
	static const double ones[6] = {1.0,1.0,1.0,1.0,1.0,1.0};
	return create_gtr("jc", &ones[0], &ones[6], &ones[0], &ones[4]);
}
template<typename It1, typename It2>
bool subst_model::create_f81(const char *, It1 first1, It1 last1, It2 first2, It2 last2) {
	// equal rates and frequencies
	static const double ones[6] = {1.0,1.0,1.0,1.0,1.0,1.0};
	return create_gtr("f81", &ones[0], &ones[6], first2, last2);
}
template<typename It1, typename It2>
bool subst_model::create_k2p(const char *, It1 first1, It1 last1, It2 first2, It2 last2) {
	// equal rates and frequencies
	static const double ones[4] = {1.0,1.0,1.0,1.0};
	double p[6], a, b=0.5;  // this default for b means that a=r if b is not specified
	if(first1 == last1)
		return DAWG_ERROR("Invalid subst model; k2p requires one or two parameters.");
	a = *first1++;
	if(first1 != last1)
		b = *first1;
	p[1] = p[4] = a;
	p[0] = p[2] = p[3] = p[5] = b;
	return create_gtr("k2p", &p[0], &p[6], &ones[0], &ones[4]);
}
template<typename It1, typename It2>
bool subst_model::create_tn(const char *mod_name, It1 first1, It1 last1, It2 first2, It2 last2) {
	double p[6], f[4], fr, fy, d, ay, ar, b;
	int u;
	u = 0;
	d = 0.0;
	// read frequencies
	if(!create_freqs(mod_name, first2, last2, &f[0], &f[4]))
		return false;
	fr = f[0]+f[2];
	fy = f[1]+f[3];
	if(first1 == last1)
		return DAWG_ERROR("Invalid subst model; " << mod_name << " tn requires two or three parameters.");
	ay = *first1++;
	if(first1 == last1)
		return DAWG_ERROR("Invalid subst model; " << mod_name << " tn requires two or three parameters.");
	ar = *first1++;
	if(first1 == last1) {
		// two parameters
		double R = ay, rho=ar;
		ay = (fr*fy*R-f[0]*f[2]-f[1]*f[3])/
		     (2.0*(1.0+R)*(fy*f[0]*f[2]*rho+fr*f[1]*f[3]));
		ar = rho*ay;
		b = 0.5/(fr*fy*(1.0+R));
		ar = ar/fr+b;
		ay = ar/fy+b;
		
	} else {
		// three parameters
		b = *first1;			
	}
	p[1] = ar;
	p[4] = ay;
	p[0] = p[2] = p[3] = p[5] = b;	
	return create_gtr(mod_name, &p[0], &p[6], &f[0], &f[4]);
}

template<typename It1, typename It2>
bool subst_model::create_tn_f04(const char *mod_name, It1 first1, It1 last1, It2 first2, It2 last2) {
	double p[6], f[4], fr, fy, d, ay, ar, b;
	int u;
	u = 0;
	d = 0.0;
	// read frequencies
	if(!create_freqs(mod_name, first2, last2, &f[0], &f[4]))
		return false;
	fr = f[0]+f[2];
	fy = f[1]+f[3];
	if(first1 == last1)
		return DAWG_ERROR("Invalid subst model; " << mod_name << " requires two or three parameters.");
	ay = *first1++;
	if(first1 == last1)
		return DAWG_ERROR("Invalid subst model; " << mod_name << " requires two or three parameters.");
	ar = *first1++;
	if(first1 == last1) {
		// two parameters
		double R = ay, rho=ar;
		ay = (fr*fy*R-f[0]*f[2]-f[1]*f[3])/
		     (2.0*(1.0+R)*(fy*f[0]*f[2]*rho+fr*f[1]*f[3]));
		ar = rho*ay;
		b = 0.5/(fr*fy*(1.0+R));
	} else {
		// three parameters
		b = *first1;			
	}
	p[1] = ar/fr+b;
	p[4] = ay/fy+b;
	p[0] = p[2] = p[3] = p[5] = b;	
	return create_gtr(mod_name, &p[0], &p[6], &f[0], &f[4]);
}

template<typename It1, typename It2>
bool subst_model::create_f84(const char *, It1 first1, It1 last1, It2 first2, It2 last2) {
	double p[3];
	if(first1 == last1)
		return DAWG_ERROR("Invalid subst model; f84 requires one or two parameters.");
	double a = *first1++;
	if(first1 == last1) {
		p[0] = a;
		p[1] = 1.0;
		return create_tn_f04("f84", &p[0], &p[2], first2, last2);
	}
	double b = *first1;
	p[0] = p[1] = a;
	p[2] = b;
	return create_tn_f04("f84", &p[0], &p[3], first2, last2);
}

template<typename It1, typename It2>
bool subst_model::create_hky(const char *, It1 first1, It1 last1, It2 first2, It2 last2) {
	double p[3];
	if(first1 == last1)
		return DAWG_ERROR("Invalid subst model; hky requires one or two parameters.");
	double a = *first1++;
	if(first1 == last1) {
		p[0] = a;
		p[1] = 1.0;
		return create_tn("hky", &p[0], &p[2], first2, last2);
	}
	double b = *first1;
	p[0] = p[1] = a;
	p[2] = b;
	return create_tn("hky", &p[0], &p[3], first2, last2);
}

};
 
#endif // DAWG_SUBST_DNA_H
