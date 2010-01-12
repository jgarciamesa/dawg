#include "app.h"

IMPLEMENT_APP( DawgGUI );

DawgGUI::DawgGUI()
{
}

bool DawgGUI::OnInit()
{
	frame  = new dawgFrame(wxT("Dawg Test Frame"));
	subst  = new dawgPage(frame, wxT("Subst"));	
	indel  = new dawgPage(frame, wxT("Indel"));	
	root   = new dawgPage(frame, wxT("Root"));	
	output = new dawgPage(frame, wxT("Ouput"));	
	sim    = new dawgPage(frame, wxT("Sim"));	
	tree   = new dawgPage(frame, wxT("Tree"));
	
	#define XM(name, atype, def) _V(name) = new dawg_gui_type< atype >::type \
		(BOOST_PP_SEQ_HEAD(name), wxT(_S(BOOST_PP_SEQ_TAIL(name))), \
		dawg_gui_type< atype >::default_type(def));
	#include "dawgma.xmh"
	#undef XM

	frame->Show(true);
	return true;
}

DawgGUI::~DawgGUI()
{
	#define XM(name, atype, def) delete _V(name);
	#include "dawgma.xmh"
	#undef XM
	delete subst, indel, root, output, sim, tree;
	delete frame;
}
