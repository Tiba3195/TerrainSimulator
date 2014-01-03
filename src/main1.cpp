#include "main.h"
#include "simulation.h"
#include "gl_object.h"
#include "Button_Bitmap_rcs\rotate_icon_png.cpp" //png bild in c vector
#include <crtdbg.h>


//#include "global_class_function.h"


//extern void glUseProgram(GLuint program);
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SIMSIZEX 256
#define SIMSIZEY 256
#define terrainStepWidth 0.0625
#define terrainStepLength 0.0625
//gibt an wie stark coeff geteilt wird
//je groesser der faktor, desto feiner kann man die faltung 
//einstellen aber desto mehr sliderplatz braucht es
#define coeff_sizer  15.0




float BasicGLPane::zoom = -5.0; // muss ausserhalb der klasse definiert werden, da statische variable
int durchlaufe = 0;

using namespace std;
int f;

AltitudeMap Alti(256,256);
BasicGLPane *glPlane; 

wxBitmap global_preview_bitmap = wxBitmap(Alti.getxsize(),Alti.getysize(),-1);
wxBitmap global_sediment_bitmap = wxBitmap(Alti.getxsize(),Alti.getysize(),-1);
BasicFrame *frame;



struct Water_Constants Water_Kcs = {
	/*Water_Kcs.KC*/ 0.01,
	/* Water_Kcs.KD*/ 0.5,
	/*Water_Kcs.KE*/0.5,
	/*Water_Kcs.KS*/ 0.01, 
	/*Water_Kcs.KR*/0.01,
	/*Water_Kcs.Pipe_length_x*/ 1.0,
	/*Water_Kcs.Pipe_length_y*/1.0,
	/*Water_Kcs.Radius*/ 50.0,
	/*Water_Kcs.Water_Amount*/  100.0,
	/*Water_Kcs.DT*/ 0.1,
	/*Water_Kcs.Min_alpha  */0.01,
	/*Water_Kcs.Rain_on */ true,
	/*Water_Kcs.Watersource_on*/ true,
	/*Iterations*/50
};



IMPLEMENT_APP(HalloWeltApp)










bool HalloWeltApp::OnInit() {
   sizer = new wxBoxSizer(wxHORIZONTAL);
   frame = new BasicFrame( wxT("Terrain Generator"), 50, 50,1024, 800);

   
	//BasicFrame *p_objekt = frame;
   
   //sizer->Add(frame, 1, wxEXPAND);

   //frame->SetSizer(sizer);
   frame->SetIcon(frame->icon_big);
   frame->Show(TRUE);
   SetTopWindow(frame);
   frame->ManualRender();
   return TRUE;
}

HalloWeltApp::~HalloWeltApp()
{ sizer->Clear();
delete sizer;}

BasicFrame::BasicFrame (
   const wxChar *title,
   int xpos, int ypos,
   int width, int height)
   : wxFrame ( (wxFrame *) NULL,
               -1, title,
               wxPoint(xpos, ypos),
			   wxSize(width, height)){ 
	
	
	Coeff = 3.0 / coeff_sizer;
	Erosion_radius = 3;
	Erosion_iterationen = 3;
	Randomize_radius = 50;
	side_window = true; // das fenster mit dem preview pic ist an
	crosshair = true;
	Perlin_Persistence = 0.25;
	Perlin_Zoom = 0.25;
	Octaves = 5;
	factor_combination1 =1;
	factor_combination2 = 1;
	factor_combination3 = 1;
	Choice_combination1 = 0;//0 = subdivision
	Choice_combination2 = 0;
	Choice_combination3 = 3;//3= None
	UpdateImage();
	
	

	wxInitAllImageHandlers(); // Alle image handlers initalisieren, um sie beim save image zu nutzen


	imagelist = new wxImageList(20,20,true,3);
	wxBitmap saveicon = wxBitmap(wxT("BITMAP_SAVE"), wxBITMAP_TYPE_RESOURCE );
	wxBitmap saveicon_mask = wxBitmap(wxT("BITMAP_SAVE_MASK"), wxBITMAP_TYPE_RESOURCE );
	wxBitmap Erosion_icon = wxBitmap(wxT("BITMAP_EROSION"),wxBITMAP_TYPE_RESOURCE );
	wxBitmap Subdivision_icon = wxBitmap(wxT("BITMAP_SUBDIVISION"), wxBITMAP_TYPE_RESOURCE );
	wxBitmap Plateau_icon = wxBitmap(wxT("BITMAP_PLATEAU"), wxBITMAP_TYPE_RESOURCE );
	wxBitmap Open_icon = wxBitmap(wxT("BITMAP_OPEN"), wxBITMAP_TYPE_RESOURCE );
	wxBitmap Open_icon_mask = wxBitmap(wxT("BITMAP_OPEN_MASK"), wxBITMAP_TYPE_RESOURCE );
	wxBitmap Scale_icon = wxBitmap(wxT("BITMAP_SCALE"), wxBITMAP_TYPE_RESOURCE );
	wxBitmap Water_icon = wxBitmap(wxT("BITMAP_WATER"),wxBITMAP_TYPE_RESOURCE );
	wxBitmap Pfeil = wxBitmap(wxT("BITMAP_PFEIL"),wxBITMAP_TYPE_RESOURCE );
	wxBitmap Pfeil_mask_bmp = wxBitmap(wxT("BITMAP_PFEIL_MASKO"),wxBITMAP_TYPE_RESOURCE );
		
	imagelist->Add(Subdivision_icon,wxColour(char(255),char(255),char(255))); //weiss ist transparent
	imagelist->Add(Erosion_icon,wxColour(char(255),char(255),char(255)));
	imagelist->Add(saveicon,wxColour(char(255),char(255),char(255)));
	imagelist->Add(Plateau_icon,wxColour(char(255),char(255),char(255)));
	imagelist->Add(Open_icon,wxColour(char(255),char(255),char(255)));
	imagelist->Add(Scale_icon,wxColour(char(255),char(255),char(255)));
	imagelist->Add(Water_icon,wxColour(char(255),char(255),char(255)));
	//imagelist->Add(Pfeil,wxColour(char(255),char(255),char(255)));


	pfeil_mask = new wxMask(Pfeil_mask_bmp);
	Pfeil.SetMask(pfeil_mask);
	Open_mask = new wxMask(Open_icon_mask);
	Open_icon.SetMask(Open_mask);
	Save_mask = new wxMask(saveicon_mask);
	saveicon.SetMask(Save_mask);

	wxBoxSizer* Super_sizer;
	Super_sizer = new wxBoxSizer( wxHORIZONTAL );
	
	m_splitter3 = new wxSplitterWindow( this, SPLITTER_WINDOW, wxDefaultPosition, wxDefaultSize, wxSP_3D|wxSP_3DBORDER|wxSP_3DSASH );
	//m_splitter3->Connect( wxEVT_IDLE, wxIdleEventHandler( BasicFrame::m_splitter3OnIdle ), NULL, this );
	
	m_panel12 = new wxPanel( m_splitter3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	//wxBoxSizer* bSizer21;
	//bSizer21 = new wxBoxSizer( wxVERTICAL );
	


	
	wxBoxSizer *generalsizer = new wxBoxSizer(wxVERTICAL);
	
	wxBoxSizer *panelsizer = new wxBoxSizer(wxVERTICAL);
	//panel = new wxPanel(this, wxID_ANY,wxPoint(0,0),wxSize(width,80));





	wxBoxSizer* bSizer5;
	m_notebook2 = new wxNotebook( m_panel12, wxID_ANY, wxDefaultPosition, wxSize( -1,50 ), 0 );
	Subdivision = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	bSizer5 = new wxBoxSizer( wxHORIZONTAL );

	m_notebook3 = new wxNotebook( Subdivision, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
	m_panel14 = new wxPanel( m_notebook3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer60;
	bSizer60 = new wxBoxSizer( wxHORIZONTAL );
	
	m_button3 = new wxButton( m_panel14, BUTTON_SUBDIVIDE, wxT("Subdivide"), wxPoint( 100,40 ), wxSize( 100,-1 ), 0 );
	m_button3->SetToolTip( wxT("Das Terrain erstellen") );
	wxBoxSizer* bSizer101;
	bSizer101 = new wxBoxSizer( wxVERTICAL );
	
	bSizer101->Add( m_button3, 0, wxALL, 5 );
	Randomize_but = new wxButton( m_panel14,BUTTON_RANDOMIZE, wxT("Randomize"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer101->Add( Randomize_but, 0, wxALL, 5 );
	
	bSizer60->Add( bSizer101, 1, wxEXPAND, 5 );

	
	
	
	//bSizer5->Add( bSizer101, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer6;
	bSizer6 = new wxBoxSizer( wxVERTICAL );

	wxBoxSizer* subdivide_coeff_sizer;
	subdivide_coeff_sizer = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText3 = new wxStaticText( m_panel14, wxID_ANY, wxT("Coeff :"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText3->Wrap( -1 );
	m_staticText3->SetFont( wxFont( -1, 70, 90, 90, false, wxEmptyString ) );
	m_staticText3->SetToolTip( wxT("Der Coeff stellt die stärke der Faltung des Geländes ein") );

	subdivide_coeff_sizer->Add( m_staticText3, 0, wxALIGN_LEFT|wxALL, 5 );
	
	//bSizer6->Add( m_staticText3, 0, wxALIGN_LEFT|wxALL, 5 );
	
	subdiv_slider_coeff =  new wxSlider( m_panel14, SLIDER_SUBDIVISION_COEFF, 5, 5, 20, wxPoint( -1,-1 ), wxDefaultSize, wxSL_AUTOTICKS|wxSL_LABELS|wxSL_RIGHT|wxSL_TOP );
	subdivide_coeff_sizer->Add(subdiv_slider_coeff, 0, wxALIGN_CENTER|wxALIGN_TOP|wxALL|wxSHAPED, 5 );
	

	
	m_staticText271 = new wxStaticText( m_panel14, wxID_ANY, wxT("Fine_coeff:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText271->Wrap( -1 );
	
	slider_fine_subdivision = new wxSlider( m_panel14, SLIDER_FINE_SUBDIV, 10, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_AUTOTICKS|wxSL_HORIZONTAL|wxSL_LABELS|wxSL_RIGHT|wxSL_TOP );
	slider_fine_subdivision->SetToolTip( wxT("Feineinstellung für den coeffizienten") );

	subdivide_coeff_sizer->Add( m_staticText271, 0, wxALL, 5 );
	subdivide_coeff_sizer->Add( slider_fine_subdivision, 0, wxALL, 5 );

	
	bSizer6->Add( subdivide_coeff_sizer, 1, wxEXPAND, 5 );

	
	wxBoxSizer* random_slider_sizer;
	random_slider_sizer = new wxBoxSizer( wxHORIZONTAL );

	randomcoeff = new wxStaticText( m_panel14, wxID_ANY, wxT("Random factor"), wxDefaultPosition, wxDefaultSize, 0 );
	randomcoeff->Wrap( -1 );
	random_slider_sizer->Add( randomcoeff, 0, wxALL, 5 );
	
	random_fac_slider = new wxSlider( m_panel14, RANDOMIZE_SLIDER, (int)Randomize_radius, 10, 100, wxDefaultPosition, wxDefaultSize, wxSL_AUTOTICKS|wxSL_HORIZONTAL|wxSL_LABELS );
	random_slider_sizer->Add( random_fac_slider, 0, wxALIGN_TOP|wxALL, 5 );

	


	//bSizer6->Add( subdiv_slider_coeff, 0, wxALIGN_CENTER|wxALIGN_TOP|wxALL|wxSHAPED, 5 );
	bSizer6->Add( random_slider_sizer, 1, wxEXPAND, 5 );

	bSizer60->Add( bSizer6, 1, wxEXPAND, 5 );
	
	m_panel14->SetSizer( bSizer60 );
	m_panel14->Layout();
	bSizer60->Fit( m_panel14 );
	m_notebook3->AddPage( m_panel14, wxT("MidPoint"), false );




	m_panel15 = new wxPanel( m_notebook3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer62;
	bSizer62 = new wxBoxSizer( wxHORIZONTAL );	
	wxBoxSizer* bSizer63;
	bSizer63 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer66;
	bSizer66 = new wxBoxSizer( wxVERTICAL );
	m_button11 = new wxButton( m_panel15, BUTTON_GENERATE, wxT("Generate "), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer66->Add( m_button11, 0, wxALIGN_CENTER|wxALL, 5 );
	bSizer63->Add( bSizer66, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer67;
	bSizer67 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText301 = new wxStaticText( m_panel15, wxID_ANY, wxT("Zoom"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText301->Wrap( -1 );
	bSizer67->Add( m_staticText301, 0, wxALL, 5 );
	m_slider211 = new wxSlider( m_panel15, SLIDER_PERLIN_ZOOM, 25, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
	bSizer67->Add( m_slider211, 0, wxALL|wxEXPAND, 5 );
	
	bSizer63->Add( bSizer67, 1, wxEXPAND, 5 );
	bSizer62->Add( bSizer63, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer64;
	bSizer64 = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer* bSizer65;
	bSizer65 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText_persistence = new wxStaticText( m_panel15, wxID_ANY, wxT("Persistence"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_persistence->Wrap( -1 );
	m_staticText_persistence->SetToolTip( wxT("Hohe werte machen rauhes, niedrige weiches terrain") );
	
	bSizer65->Add( m_staticText_persistence, 0, wxALL, 5 );
	m_slider201 = new wxSlider( m_panel15, SLIDER_PERSISTENCE, 25, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_AUTOTICKS|wxSL_HORIZONTAL|wxSL_LABELS );
	bSizer65->Add( m_slider201, 0, wxALL|wxEXPAND, 5 );
	
	m_staticline4 = new wxStaticLine( m_panel15, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer65->Add( m_staticline4, 0, wxEXPAND | wxALL, 5 );
	
	m_staticText29 = new wxStaticText( m_panel15, wxID_ANY, wxT("Octaves"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText29->Wrap( -1 );
	bSizer65->Add( m_staticText29, 0, wxALL, 5 );
	
	m_spinCtrl1 = new wxSpinCtrl( m_panel15, SLIDER_OCTAVES, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 12, 5);
	bSizer65->Add( m_spinCtrl1, 0, wxALL, 5 );
	
	bSizer64->Add( bSizer65, 1, wxEXPAND, 5 );
	bSizer62->Add( bSizer64, 1, wxEXPAND, 5 );	
	m_panel15->SetSizer( bSizer62 );
	m_panel15->Layout();
	bSizer62->Fit( m_panel15 );
	m_notebook3->AddPage( m_panel15, wxT("Perlin"), true );

	//--------------Voronoi-----------------
	m_voronoi = new wxPanel( m_notebook3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer651;
	bSizer651 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer661;
	bSizer661 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer70;
	bSizer70 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer75;
	bSizer75 = new wxBoxSizer( wxHORIZONTAL );
	
	m_button_voro_gen = new wxButton( m_voronoi, BUTTON_VORONOI, wxT("Generate"), wxDefaultPosition, wxDefaultSize, 0 );
	m_button_voro_gen->SetToolTip( wxT("Zellenartige struktur erstellen") );
	
	bSizer75->Add( m_button_voro_gen, 0, wxALL, 5 );
	
	m_checkBox_negative = new wxCheckBox( m_voronoi, wxID_ANY, wxT("Negative"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer75->Add( m_checkBox_negative, 0, wxALL, 10 );
	
	bSizer70->Add( bSizer75, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer76;
	bSizer76 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText_voronoi_coeffs = new wxStaticText( m_voronoi, wxID_ANY, wxT("Coefficients"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_voronoi_coeffs->Wrap( -1 );
	bSizer76->Add( m_staticText_voronoi_coeffs, 0, wxALL, 5 );
	
	m_textCtrl2_vor_coeff = new wxTextCtrl( m_voronoi, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	wxTextValidator Coeff_validator =  wxTextValidator( wxFILTER_INCLUDE_CHAR_LIST, &voronoi_coeff_validator );
	wxString includes[] = {wxT("1"),wxT("2"),wxT("3"),wxT("4"),wxT("5"),wxT("6"),wxT("7"),wxT("8"),wxT("9"),wxT("0"),wxT("."),wxT(";"),wxT("-")};
	Coeff_validator.SetIncludes( wxArrayString(13,includes) );
	m_textCtrl2_vor_coeff->SetValidator( Coeff_validator  );
	m_textCtrl2_vor_coeff->SetToolTip( wxT("Koeffizienten, die die Form beeinflussen, getrennt durch ';'") );
	
	bSizer76->Add( m_textCtrl2_vor_coeff, 0, wxALL, 5 );
	
	bSizer70->Add( bSizer76, 1, wxEXPAND, 5 );
	
	bSizer661->Add( bSizer70, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer71;
	bSizer71 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer72;
	bSizer72 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText_distances = new wxStaticText( m_voronoi, wxID_ANY, wxT("Distance Methods"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_distances->Wrap( -1 );
	bSizer72->Add( m_staticText_distances, 0, wxALL, 5 );
	
	wxString m_choice_distanceChoices[] = { wxT("Linear"), wxT("SquareLength"), wxT("Manhattan"), wxT("Chebyshev"), wxT("Quadratic"),wxT("Meine Methode") };
	int m_choice_distanceNChoices = sizeof( m_choice_distanceChoices ) / sizeof( wxString );
	m_choice_distance = new wxChoice( m_voronoi, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice_distanceNChoices, m_choice_distanceChoices, 0 );
	m_choice_distance->SetSelection( 0 );
	m_choice_distance->SetToolTip( wxT("abstand algorithmen, linear ist am langsamsten") );
	
	bSizer72->Add( m_choice_distance, 0, wxALL, 5 );
	
	bSizer71->Add( bSizer72, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer73;
	bSizer73 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText_clip = new wxStaticText( m_voronoi, wxID_ANY, wxT("Clip"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_clip->Wrap( -1 );
	m_staticText_clip->SetToolTip( wxT("wie gross der kontrast ist, werte zwischen 0 und 1") );
	
	bSizer73->Add( m_staticText_clip, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer74;
	bSizer74 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText35 = new wxStaticText( m_voronoi, wxID_ANY, wxT("0."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText35->Wrap( -1 );
	bSizer74->Add( m_staticText35, 0, wxLEFT|wxTOP, 10 );
	
	m_textCtrl1 = new wxTextCtrl( m_voronoi, wxID_ANY, wxT("0"), wxDefaultPosition, wxDefaultSize, wxTE_NO_VSCROLL|wxSIMPLE_BORDER );
	m_textCtrl1->SetMaxLength( 3 ); 
	m_textCtrl1->SetValidator( wxTextValidator( wxFILTER_NUMERIC, &clip_variable ) );
	m_textCtrl1->SetFont( wxFont( 8, 70, 90, 90, false, wxEmptyString ) );
	m_textCtrl1->SetToolTip( wxT("wie gross der kontrast ist, werte zwischen 0 und 1") );
	
	bSizer74->Add( m_textCtrl1, 0, wxTOP, 8 );
	
	bSizer73->Add( bSizer74, 1, wxEXPAND, 5 );
	
	bSizer71->Add( bSizer73, 1, wxEXPAND, 5 );
	
	bSizer661->Add( bSizer71, 1, wxEXPAND, 5 );
	
	bSizer651->Add( bSizer661, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer671;
	bSizer671 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer68;
	bSizer68 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText_blocksize = new wxStaticText( m_voronoi, wxID_ANY, wxT("BlockSize"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_blocksize->Wrap( -1 );
	bSizer68->Add( m_staticText_blocksize, 0, wxALL, 5 );
	
	m_slider_blocksize = new wxSlider( m_voronoi, wxID_ANY, 20, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
	m_slider_blocksize->SetToolTip( wxT("groesse der einzelnen zellen, je groesser desto mehr randterrain muss berechnet werden") );
	
	bSizer68->Add( m_slider_blocksize, 0, wxEXPAND|wxRIGHT, 10 );
	
	m_staticline5 = new wxStaticLine( m_voronoi, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer68->Add( m_staticline5, 0, wxEXPAND | wxALL, 2 );
	
	bSizer671->Add( bSizer68, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer69;
	bSizer69 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText_fValue = new wxStaticText( m_voronoi, wxID_ANY, wxT("Intesity"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_fValue->Wrap( -1 );
	bSizer69->Add( m_staticText_fValue, 0, wxALL, 1 );
	
	m_slider_fValue = new wxSlider( m_voronoi, wxID_ANY, 1, 1, 7, wxDefaultPosition, wxDefaultSize, wxSL_AUTOTICKS|wxSL_HORIZONTAL|wxSL_LABELS );
	m_slider_fValue->SetToolTip( wxT("Intensität der zellwände") );
	
	bSizer69->Add( m_slider_fValue, 0, wxEXPAND|wxRIGHT, 10 );
	
	bSizer671->Add( bSizer69, 1, wxEXPAND, 5 );
	
	bSizer651->Add( bSizer671, 1, wxEXPAND, 5 );
	
	m_voronoi->SetSizer( bSizer651 );
	m_voronoi->Layout();
	bSizer651->Fit( m_voronoi );
	m_notebook3->AddPage( m_voronoi, wxT("Voronoi"), true );

//#-----------------Combined Panel -------------------------------------------#//
		Combined_panel = new wxPanel( m_notebook3, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	Combined_panel->SetToolTip( wxT("Den Faktor einstellen, mit der diese Noise methode gemischt wird") );
	
	wxBoxSizer* bSizer77;
	bSizer77 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer80;
	bSizer80 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer81;
	bSizer81 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer98;
	bSizer98 = new wxBoxSizer( wxVERTICAL );
	
	wxString m_choice21Choices[] = { wxT("Midpoint Displacement"), wxT("Perlin"), wxT("Voronoi") };
	int m_choice21NChoices = sizeof( m_choice21Choices ) / sizeof( wxString );
	m_choice21 = new wxChoice( Combined_panel, CHOICEBOX1, wxDefaultPosition, wxDefaultSize, m_choice21NChoices, m_choice21Choices, 0 );
	m_choice21->SetSelection( 0 );
	bSizer98->Add( m_choice21, 0, wxALL, 5 );
	
	m_bitmap3 = new wxStaticBitmap( Combined_panel, wxID_ANY, Pfeil, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer98->Add( m_bitmap3, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	bSizer81->Add( bSizer98, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer99;
	bSizer99 = new wxBoxSizer( wxVERTICAL );
	
	m_spinCtrl2_add_factor_1 = new wxSpinCtrl( Combined_panel, SPINCTRL1, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 1, 10, 1 );
	m_spinCtrl2_add_factor_1->SetValidator( wxTextValidator( wxFILTER_NUMERIC, &combination_listbox1 ) );
	m_spinCtrl2_add_factor_1->SetToolTip( wxT("Den Faktor einstellen, mit der diese Noise methode gemischt wird") );
	
	bSizer99->Add( m_spinCtrl2_add_factor_1, 0, wxTOP|wxRIGHT|wxLEFT, 5 );
	
	bSizer81->Add( bSizer99, 0, wxEXPAND, 5 );
	
	bSizer80->Add( bSizer81, 0, 0, 5 );
	
	wxBoxSizer* bSizer82;
	bSizer82 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer85;
	bSizer85 = new wxBoxSizer( wxVERTICAL );
	
	wxString m_choice2Choices[] = { wxT("Midpoint Displacement"), wxT("Perlin"), wxT("Voronoi"), wxT("None") };
	int m_choice2NChoices = sizeof( m_choice2Choices ) / sizeof( wxString );
	m_choice2 = new wxChoice( Combined_panel, CHOICEBOX2, wxDefaultPosition, wxDefaultSize, m_choice2NChoices, m_choice2Choices, 0 );
	m_choice2->SetSelection( 0 );
	bSizer85->Add( m_choice2, 0, wxALL, 5 );
	
	m_bitmap4 = new wxStaticBitmap( Combined_panel, wxID_ANY, Pfeil, wxDefaultPosition, wxDefaultSize, 0 );
	bSizer85->Add( m_bitmap4, 0, wxALIGN_CENTER, 5 );
	
	bSizer82->Add( bSizer85, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer97;
	bSizer97 = new wxBoxSizer( wxVERTICAL );
	
	m_spinCtrl2_add_factor_2 = new wxSpinCtrl( Combined_panel, SPINCTRL2, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 10, 1 );
	m_spinCtrl2_add_factor_2->SetValidator( wxTextValidator( wxFILTER_NUMERIC, &combination_listbox2 ) );
	m_spinCtrl2_add_factor_2->SetToolTip( wxT("Den Faktor einstellen, mit der diese Noise methode gemischt wird") );
	
	bSizer97->Add( m_spinCtrl2_add_factor_2, 0, wxALL|wxEXPAND, 5 );
	
	bSizer82->Add( bSizer97, 0, wxEXPAND, 0 );
	
	bSizer80->Add( bSizer82, 0, 0, 5 );
	
	wxBoxSizer* bSizer83;
	bSizer83 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer1011;
	bSizer1011 = new wxBoxSizer( wxVERTICAL );
	
	wxString m_choice22Choices[] = { wxT("Midpoint Displacement"), wxT("Perlin"), wxT("Voronoi"), wxT("None") };
	int m_choice22NChoices = sizeof( m_choice22Choices ) / sizeof( wxString );
	m_choice22 = new wxChoice( Combined_panel, CHOICEBOX3, wxDefaultPosition, wxDefaultSize, m_choice22NChoices, m_choice22Choices, 0 );
	m_choice22->SetSelection( 3 );
	bSizer1011->Add( m_choice22, 0, wxALL, 5 );
	
	bSizer83->Add( bSizer1011, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer100;
	bSizer100 = new wxBoxSizer( wxVERTICAL );
	
	m_spinCtrl2_add_factor_3 = new wxSpinCtrl( Combined_panel, SPINCTRL3, wxEmptyString, wxDefaultPosition, wxSize( 100,-1 ), wxSP_ARROW_KEYS|wxSP_WRAP, 0, 10, 0 );
	m_spinCtrl2_add_factor_3->SetValidator( wxTextValidator( wxFILTER_NUMERIC, &combination_listbox3 ) );
	
	bSizer100->Add( m_spinCtrl2_add_factor_3, 0, wxALL, 5 );
	
	bSizer83->Add( bSizer100, 0, wxEXPAND, 5 );
	
	bSizer80->Add( bSizer83, 0, wxEXPAND, 5 );
	
	bSizer77->Add( bSizer80, 1, wxEXPAND, 5 );
	
	m_staticline8 = new wxStaticLine( Combined_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	bSizer77->Add( m_staticline8, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer84;
	bSizer84 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer86;
	bSizer86 = new wxBoxSizer( wxHORIZONTAL );
	
	m_button_comb_generate = new wxButton( Combined_panel, COMBINED_GENERATE, wxT("Generate"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer86->Add( m_button_comb_generate, 1, wxALL|wxEXPAND, 5 );
	
	m_staticline6 = new wxStaticLine( Combined_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	bSizer86->Add( m_staticline6, 0, wxEXPAND | wxALL, 5 );
	
	m_button_perturb = new wxButton( Combined_panel, BUTTON_PERTURB, wxT("Perturb"), wxDefaultPosition, wxDefaultSize, 0 );
	m_button_perturb->SetToolTip( wxT("Wende Perturbation filter auf existierende Karte an") );
	
	bSizer86->Add( m_button_perturb, 1, wxALL|wxEXPAND, 5 );
	
	bSizer84->Add( bSizer86, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer87;
	bSizer87 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer92;
	bSizer92 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText37 = new wxStaticText( Combined_panel, wxID_ANY, wxT("Load / Save Presets:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText37->Wrap( -1 );
	bSizer92->Add( m_staticText37, 1, wxALL, 5 );
	
	
	bSizer92->Add( 0, 0, 0, wxEXPAND, 5 );
	
	m_staticText39 = new wxStaticText( Combined_panel, wxID_ANY, wxT("Noise Map:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText39->Wrap( -1 );
	m_staticText39->SetToolTip( wxT("Die Noise Funktion für den Perturbation filter") );
	
	bSizer92->Add( m_staticText39, 1, wxALL, 5 );
	
	bSizer87->Add( bSizer92, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer93;
	bSizer93 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer88;
	bSizer88 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer90;
	bSizer90 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer911;
	bSizer911 = new wxBoxSizer( wxHORIZONTAL );
	
	m_bpButton_load_preset = new wxBitmapButton( Combined_panel, OPEN_BUT_PRESET, Open_icon, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	bSizer911->Add( m_bpButton_load_preset, 0, wxALL, 5 );
	
	m_bpButton9_save_preset = new wxBitmapButton( Combined_panel, SAVE_BUT_PRESET, saveicon, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	bSizer911->Add( m_bpButton9_save_preset, 0, wxALL, 5 );
	
	bSizer90->Add( bSizer911, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer89;
	bSizer89 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText_load_path = new wxStaticText( Combined_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText_load_path->Wrap( -1 );
	bSizer89->Add( m_staticText_load_path, 0, wxALL, 9 );
	
	bSizer90->Add( bSizer89, 0, wxEXPAND, 5 );
	
	bSizer88->Add( bSizer90, 19, wxEXPAND, 0 );
	
	wxBoxSizer* bSizer91;
	bSizer91 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticline7 = new wxStaticLine( Combined_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	bSizer91->Add( m_staticline7, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer901;
	bSizer901 = new wxBoxSizer( wxVERTICAL );
	
	m_radioBtn1_midpoint_displ = new wxRadioButton( Combined_panel, wxID_ANY, wxT("Midpoint displ."), wxDefaultPosition, wxDefaultSize,0 );
	bSizer901->Add( m_radioBtn1_midpoint_displ, 0, wxALL, 5 );
	
	m_radioBtn2 = new wxRadioButton( Combined_panel, wxID_ANY, wxT("Perlin Noise"), wxDefaultPosition, wxDefaultSize, 0 );
	m_radioBtn2->SetValue(true);
	bSizer901->Add( m_radioBtn2, 0, wxALL, 5 );
	
	bSizer91->Add( bSizer901, 4, wxEXPAND, 0 );
	
	bSizer88->Add( bSizer91, 20, wxEXPAND, 0 );
	
	bSizer93->Add( bSizer88, 1, wxEXPAND, 5 );
	
	bSizer87->Add( bSizer93, 1, wxEXPAND, 5 );
	
	bSizer84->Add( bSizer87, 2, wxEXPAND, 5 );
	
	bSizer77->Add( bSizer84, 1, wxEXPAND, 5 );
	
	Combined_panel->SetSizer( bSizer77 );
	Combined_panel->Layout();
	bSizer77->Fit( Combined_panel );
	m_notebook3->AddPage( Combined_panel, wxT("Combined"), true );
	
	//-------------------notebook zum sizer hinzufuegen-----------

	bSizer5->Add( m_notebook3, 1, wxEXPAND | wxALL, 5 );


	//bSizer5->Add( bSizer6, 1, wxEXPAND, 5 );
	
	Subdivision->SetSizer( bSizer5 );
	Subdivision->Layout();
	bSizer5->Fit( Subdivision );
	m_notebook2->AddPage( Subdivision, wxT("Subdivsion"), true );

	m_panel5 = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );





	//######### Erosion ################### //

	wxBoxSizer* bSizer991;
	bSizer991 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer105;
	bSizer105 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer7;
	bSizer7 = new wxBoxSizer( wxHORIZONTAL );
	
	m_button4 = new wxButton( m_panel5, BUTTON_EROSION, wxT("Erosion"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer7->Add( m_button4, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer8;
	bSizer8 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText4 = new wxStaticText( m_panel5, wxID_ANY, wxT("Radius:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText4->Wrap( -1 );
	m_staticText4->SetToolTip( wxT("Der Radius des Effekts") );
	
	bSizer8->Add( m_staticText4, 0, wxALL, 5 );
	
	erosion_slider_radius = new wxSlider( m_panel5, EROSION_RADIUS_SLIDER, 3, 0, 5, wxDefaultPosition, wxDefaultSize, wxSL_AUTOTICKS|wxSL_HORIZONTAL|wxSL_LABELS );
	bSizer8->Add( erosion_slider_radius, 0, wxALL, 5 );
	
	
	bSizer8->Add( 0, 0, 1, wxEXPAND, 5 );
	
	bSizer7->Add( bSizer8, 1, wxEXPAND, 5 );
	
	bSizer105->Add( bSizer7, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer102;
	bSizer102 = new wxBoxSizer( wxVERTICAL );
	
	m_staticline9 = new wxStaticLine( m_panel5, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer102->Add( m_staticline9, 0, wxALL | wxEXPAND, 5 );
	
	wxBoxSizer* bSizer103;
	bSizer103 = new wxBoxSizer( wxHORIZONTAL );
	
	m_button_thermal_erosion = new wxButton( m_panel5, BUTTON_THERMAL_EROSION, wxT("Thermal Erosion"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer103->Add( m_button_thermal_erosion, 0, wxALL, 5 );
	
	m_staticText40 = new wxStaticText( m_panel5, wxID_ANY, wxT("Talus threshold:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText40->Wrap( -1 );
	m_staticText40->SetToolTip( wxT("Grenze, überhalb der Material verschoben wird") );
	
	bSizer103->Add( m_staticText40, 0, wxALL, 5 );
	
	m_staticText41 = new wxStaticText( m_panel5, wxID_ANY, wxT("0."), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText41->Wrap( -1 );
	bSizer103->Add( m_staticText41, 0, wxLEFT|wxTOP, 5 );
	
	m_textCtrl3 = new wxTextCtrl( m_panel5, TOGGLE_AUTO, wxT("015625"), wxDefaultPosition, wxDefaultSize, 0 );
	m_textCtrl3->SetValidator( wxTextValidator( wxFILTER_NUMERIC, &threshold_var ) );
	
	bSizer103->Add( m_textCtrl3, 0, wxRIGHT|wxSHAPED|wxTOP, 3 );
	
	m_toggleBtn5 = new wxToggleButton( m_panel5, TOGGLE_AUTO, wxT("Auto"), wxDefaultPosition, wxDefaultSize, 0 );
	m_toggleBtn5->SetValue( false ); 
	m_toggleBtn5->SetToolTip( wxT("Automatisch den wert 4/xsize als Talus threshold nehmen") );
	m_toggleBtn5->SetMinSize( wxSize( 40,-1 ) );
	
	bSizer103->Add( m_toggleBtn5, 0, 0, 0 );
	
	bSizer102->Add( bSizer103, 1, wxEXPAND, 5 );
	
	bSizer105->Add( bSizer102, 1, wxLEFT, 5 );
	
	bSizer991->Add( bSizer105, 1, wxEXPAND, 5 );
	
	
	bSizer991->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticline10 = new wxStaticLine( m_panel5, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	bSizer991->Add( m_staticline10, 0, wxALL|wxEXPAND, 5 );
	
	wxBoxSizer* bSizer106;
	bSizer106 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer104;
	bSizer104 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText5 = new wxStaticText( m_panel5, wxID_ANY, wxT("Iterationen"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText5->Wrap( -1 );
	m_staticText5->SetToolTip( wxT("Die Anzahl der Durchläufe") );
	
	bSizer104->Add( m_staticText5, 0, wxALL, 5 );
	
	erosion_slider_iterationen = new wxSlider( m_panel5, EROSION_ITER_SLIDER, 3, 0, 50, wxDefaultPosition, wxDefaultSize, wxSL_AUTOTICKS|wxSL_HORIZONTAL|wxSL_LABELS );
	bSizer104->Add( erosion_slider_iterationen, 0, wxALL, 5 );
	
	bSizer106->Add( bSizer104, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL|wxEXPAND, 5 );
	
	bSizer991->Add( bSizer106, 1, wxALIGN_CENTER, 5 );
	
	m_panel5->SetSizer( bSizer991 );
	m_panel5->Layout();
	bSizer991->Fit( m_panel5 );
	m_notebook2->AddPage( m_panel5, wxT("Erosion"), true );


	
	//####### Plateau ###### //
	
	m_panel6 = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer9;
	bSizer9 = new wxBoxSizer( wxHORIZONTAL );

	
	
	m_button7 = new wxButton( m_panel6, BUTTON_PLATEAU, wxT("Plateau"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer9->Add( m_button7, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer10;
	bSizer10 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText6 = new wxStaticText( m_panel6, wxID_ANY, wxT("Radius:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText6->Wrap( -1 );
	bSizer10->Add( m_staticText6, 0, wxALL, 10 );
	
	m_slider6 = new wxSlider( m_panel6, PLATEAU_RADIUS_SLIDER, 5, 1,( Alti.getxsize() >=  Alti.getysize()) ? ( Alti.getxsize() - 1) :  ( Alti.getysize() - 1) , wxDefaultPosition, wxDefaultSize, wxSL_AUTOTICKS|wxSL_HORIZONTAL|wxSL_LABELS );
	bSizer10->Add( m_slider6, 0, wxALL, 5 ); 

	positionflagbitmap.LoadFile(wxT("BITMAP_POSITION_FLAG"),wxBITMAP_TYPE_BMP_RESOURCE);
	positionflagbitmap_selected.LoadFile(wxT("BITMAP_POSITION_FLAG_SELECTED"),wxBITMAP_TYPE_BMP_RESOURCE);
	
	positionflag = new wxBitmapButton( m_panel6, BITMAP_BUTTON_FLAGPOS, positionflagbitmap_selected, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	positionflag->SetToolTip( wxT("Die Position des Plateaus setzen") );
	bSizer10->Add( positionflag, 0, wxALIGN_TOP|wxALIGN_RIGHT|wxALL, 5 );

	flag_pos_text = new wxStaticText( m_panel6, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0 );
	flag_pos_text->Wrap( -1 );
	bSizer10->Add( flag_pos_text, 0, wxALL, 5 );
	
	bSizer9->Add( bSizer10, 1, wxEXPAND, 5 );
	
	m_panel6->SetSizer( bSizer9 );
	m_panel6->Layout();
	bSizer9->Fit( m_panel6 );
	m_notebook2->AddPage( m_panel6, wxT("Plateau"), false );

	// ######## Load Image ####### //
	m_panel7 = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer11;
	bSizer11 = new wxBoxSizer( wxVERTICAL );
	
	m_button8 = new wxButton( m_panel7, BUTTON_OPEN_IMAGE, wxT("Load from Image"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer11->Add( m_button8, 0, wxALIGN_CENTER|wxALL|wxEXPAND, 5 );



	//load_obj_panel = new wxPanel( m_notebook2,wxID_ANY , wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer17;
	bSizer17 = new wxBoxSizer( wxVERTICAL );
	
	load_obj = new wxButton(  m_panel7,BUTTON_OPEN_OBJ , wxT("load .obj Data"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer17->Add( load_obj, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 5 );
	
	bSizer11->Add( bSizer17, 1, wxEXPAND, 5 );
	
	m_panel7->SetSizer( bSizer11 );
	m_panel7->Layout();
	bSizer11->Fit( m_panel7 );
	m_notebook2->AddPage( m_panel7, wxT("Load Image"), false );


	//##### Load .obj #### //

	//load_obj_panel->SetSizer( bSizer17 );
	//load_obj_panel->Layout();
	//bSizer17->Fit( load_obj_panel );
	//m_notebook2->AddPage( load_obj_panel, wxT("load .obj data"), false );



	//##### .obj und .tga speichern#####///
	save_files = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer18;
	bSizer18 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer19;
	bSizer19 = new wxBoxSizer( wxVERTICAL );
	
	save_obj_but = new wxButton( save_files, SAVE_OBJ, wxT("Save to .obj"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer19->Add( save_obj_but, 0, wxALL, 5 );
	
	bSizer18->Add( bSizer19, 1, wxEXPAND, 5 );
	
	m_staticline2 = new wxStaticLine( save_files, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer18->Add( m_staticline2, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer20;
	bSizer20 = new wxBoxSizer( wxVERTICAL );
	
	save_tga_but = new wxButton( save_files, SAVE_IMAGE, wxT("Save as Image"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer20->Add( save_tga_but, 0, wxALL, 5 );
	
	bSizer18->Add( bSizer20, 1, wxEXPAND, 5 );
	
	save_files->SetSizer( bSizer18 );
	save_files->Layout();
	bSizer18->Fit( save_files );
	m_notebook2->AddPage( save_files, wxT("save file"), false);

	//### scale ##///
	Scale = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer23;
	bSizer23 = new wxBoxSizer( wxVERTICAL );



		wxBoxSizer* Sizer_load_image_scale;
	Sizer_load_image_scale = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText7 = new wxStaticText( Scale, wxID_ANY, wxT("Scale Terrainheight"), wxDefaultPosition, wxDefaultSize, 0 );
	//m_staticText7->Wrap( -1 );
	m_staticText7->SetToolTip( wxT("Only affects the visual GL window, not the actual hight") );
	Sizer_load_image_scale->Add( m_staticText7, 1, wxALL, 5 );
	
	scale_image_terrain = new wxSlider( Scale, SLIDER_SCALE_IMAGE_TERRAIN, 5, 1, 10, wxDefaultPosition, wxDefaultSize, wxSL_AUTOTICKS|wxSL_HORIZONTAL|wxSL_LABELS );
	Sizer_load_image_scale->Add( scale_image_terrain, 2, wxLEFT|wxRIGHT, 20 );
	
	bSizer23->Add( Sizer_load_image_scale, 1, wxEXPAND|wxTOP, 5 );
	
	wxBoxSizer* bSizer27;
	bSizer27 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer28;
	bSizer28 = new wxBoxSizer( wxHORIZONTAL);
	
	
	
	m_scale_terrain_but = new wxButton( Scale, BUTTON_SCALE_TERRAIN_REAL, wxT("Scale Terrain"), wxDefaultPosition, wxDefaultSize, 0 );
	m_scale_terrain_but->SetToolTip( wxT("Scale the actual Terrain") );
	
	bSizer28->Add( m_scale_terrain_but, 0, wxALL, 5 );
	bSizer28->Add( 30, 0, 0, 0, 3 ); //scaler
	
	m_staticText_scale = new wxStaticText( Scale, wxID_ANY, wxT("Size: 256 x 256"), wxDefaultPosition, wxDefaultSize, 0 );
	//m_staticText_scale->Wrap( -1 );
	bSizer28->Add( m_staticText_scale, 0, wxALIGN_CENTER|wxALL, 5 );
	
	bSizer27->Add( bSizer28, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer24;
	bSizer24 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer25;
	bSizer25 = new wxBoxSizer( wxVERTICAL );
	
	m_scale_terrain = new wxSlider( Scale, SLIDER_SCALE_TERRAIN_REAL, 3, 0, 7, wxDefaultPosition, wxSize( 210,-1 ), wxSL_AUTOTICKS|wxSL_HORIZONTAL );
	bSizer25->Add( m_scale_terrain, 0, wxALL, 5 );
	
	bSizer24->Add( bSizer25, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer261;
	bSizer261 = new wxBoxSizer( wxVERTICAL );
	
	slider_labels = new wxStaticText( Scale, wxID_ANY, wxT("0.25 0.5 0.75  1.0  1.5  2.0  3.0  4.0"), wxDefaultPosition, wxDefaultSize, 0 );
	slider_labels->Wrap( -1 );
	bSizer261->Add( slider_labels, 0, wxALL, 5 );
	
	bSizer24->Add( bSizer261, 1, wxEXPAND, 5 );
	
	bSizer27->Add( bSizer24, 1, wxEXPAND, 5 );
	
	bSizer23->Add( bSizer27, 1, wxEXPAND, 5 );
	
	Scale->SetSizer( bSizer23 );
	Scale->Layout();
	bSizer23->Fit( Scale );
	m_notebook2->AddPage( Scale, wxT("Scale"), false );


 //########## Water_sim panel ########## //
	
	Water_sim_panel = new wxPanel( m_notebook2, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer271;
	bSizer271 = new wxBoxSizer( wxHORIZONTAL );

	m_notebook21 = new wxNotebook( Water_sim_panel,wxID_ANY , wxDefaultPosition, wxDefaultSize, 0 );
	water_pan_1 = new wxPanel( m_notebook21, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer43;
	bSizer43 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer44;
	bSizer44 = new wxBoxSizer( wxVERTICAL );
	
	Simulate_button = new wxButton( water_pan_1, BUTTON_WATER_SIMULATE, wxT("Simulate!"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer44->Add( Simulate_button, 0, wxALL, 5 );

		
	wxString m_radioBox1_water_methodsChoices[] = { wxT("shallow water"),wxT("strict solver"), wxT("schnell "), wxT("hydraulic erosion") };
	int m_radioBox1_water_methodsNChoices = sizeof( m_radioBox1_water_methodsChoices ) / sizeof( wxString );
	m_radioBox1_water_methods = new wxRadioBox( water_pan_1, wxID_ANY, wxT("Methods"), wxDefaultPosition, wxDefaultSize, m_radioBox1_water_methodsNChoices, m_radioBox1_water_methodsChoices, 1, wxRA_SPECIFY_COLS );
	m_radioBox1_water_methods->SetSelection( 3 );
	m_radioBox1_water_methods->SetItemToolTip(0,wxT("shallow water: Pipe reflux model, ableitung der shallow water equations, leider nicht gut für die Erosion geeignet"));
	m_radioBox1_water_methods->SetItemToolTip(1,wxT("strict solver: Erosion, streng nach dem procedural terrain generation papier. Schlechte Ergebnisse, für bessere ergebnisse nimm Hydraulische Erosion."));
	m_radioBox1_water_methods->SetItemToolTip(2,wxT("schnell -Kaputtt!-:\nSchnelle Hydraulische Erosion, leider nicht funktionsfähig"));
	m_radioBox1_water_methods->SetItemToolTip(3,wxT("Hydraulische Erosion: Die am besten funktionierened Funktion, aus \"procedural Terrain generation\"  jedoch etwas abgewandelt und  vereinfacht "));
	//m_radioBox1_water_methods->SetToolTip( wxT("Die Methoden:\n\n\n\n\n\nHydraulische Erosion: Die am besten funktionierened Funktion, aus \"procedural Terrain generation\"\n") );
	
	bSizer44->Add( m_radioBox1_water_methods, 0, 0, 5 );

	
	bSizer43->Add( bSizer44, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer45;
	bSizer45 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer46;
	bSizer46 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticline3 = new wxStaticLine( water_pan_1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	bSizer46->Add( m_staticline3, 0, wxEXPAND | wxALL, 5 );
	
	wxBoxSizer* bSizer51;
	bSizer51 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText17 = new wxStaticText( water_pan_1, wxID_ANY, wxT("Amount of rain:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText17->Wrap( -1 );
	bSizer51->Add( m_staticText17, 0, wxALL, 5 );
	
	m_togglerain = new wxToggleButton( water_pan_1, TOGGLE_RAIN, wxT("Rain"), wxDefaultPosition, wxDefaultSize, 0 );
	m_togglerain->SetValue( true ); 
	bSizer51->Add( m_togglerain, 2, wxEXPAND|wxLEFT|wxRIGHT, 4 );
	
	
	bSizer51->Add( 0, 0, 1, wxEXPAND, 2 );
	
	bSizer46->Add( bSizer51, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer52;
	bSizer52 = new wxBoxSizer( wxHORIZONTAL );
	
	//rain amount
	m_slider12 = new wxSlider( water_pan_1, SLIDER_RAIN_WATER_AMOUNT, 10, 1, 10000, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
	m_slider12->SetToolTip( wxT("scales the amount of rain per time t") );
	bSizer52->Add( m_slider12, 0, wxALL, 5 );
	
	m_staticline21 = new wxStaticLine( water_pan_1, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL );
	bSizer52->Add( m_staticline21, 0, wxEXPAND | wxALL, 2 );
	
	bSizer46->Add( bSizer52, 1, wxEXPAND, 5 );
	
	bSizer45->Add( bSizer46, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer47;
	bSizer47 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer49;
	bSizer49 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText18 = new wxStaticText( water_pan_1, wxID_ANY, wxT("Water amount:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText18->Wrap( -1 );
	bSizer49->Add( m_staticText18, 0, wxALL, 5 );
	
	m_staticText20 = new wxStaticText( water_pan_1, wxID_ANY, wxT("\n \n Source radius:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText20->Wrap( -1 );
	bSizer49->Add( m_staticText20, 0, wxALL, 5 );
	
	m_toggleBtn2 = new wxToggleButton( water_pan_1, TOGGLE_WATERSOURCE, wxT("Water source"), wxDefaultPosition, wxDefaultSize, 0 );
	m_toggleBtn2->SetValue( true ); 
	bSizer49->Add( m_toggleBtn2, 1, wxALL, 0 );
	
	bSizer47->Add( bSizer49, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer50;
	bSizer50 = new wxBoxSizer( wxVERTICAL );
	
	//Water amount
	m_slider13 = new wxSlider( water_pan_1, SLIDER_WATER_AMOUNT, 100, 1, 5000, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS|wxSL_TOP );
	m_slider13->SetToolTip( wxT("water amount in l") );
	bSizer50->Add( m_slider13, 0, wxALL, 5 );
	
	//Water radius
	m_slider14 = new wxSlider( water_pan_1, SLIDER_WATER_RADIUS, 50, 0, 100, wxPoint( -1,-1 ), wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS|wxSL_TOP );
	m_slider14->SetToolTip( wxT("radius of water source in cells") );
	bSizer50->Add( m_slider14, 0, wxALL, 5 );
	
	bSizer47->Add( bSizer50, 1, wxEXPAND, 5 );
	
	bSizer45->Add( bSizer47, 1, wxEXPAND, 5 );
	
	bSizer43->Add( bSizer45, 1, wxEXPAND, 5 );
	
	water_pan_1->SetSizer( bSizer43 );
	water_pan_1->Layout();
	bSizer43->Fit( water_pan_1 );
	m_notebook21->AddPage( water_pan_1, wxT("main"), false );
	pan_coefficients = new wxPanel( m_notebook21, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer41;
	bSizer41 = new wxBoxSizer( wxHORIZONTAL );
	
	wxBoxSizer* bSizer281;
	bSizer281 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer29;
	bSizer29 = new wxBoxSizer( wxHORIZONTAL );
	
	//sediment capacity
	m_staticText10 = new wxStaticText( pan_coefficients, wxID_ANY, wxT("sediment capacity"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText10->Wrap( -1 );
	bSizer29->Add( m_staticText10, 0, wxALL, 5 );
	
	
	m_slider8_KC = new wxSlider( pan_coefficients, SLIDER_SEDIMENT_CAPACITY, 15, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
	m_slider8_KC->SetToolTip( wxT("How much sediment the water can carry") );
	bSizer29->Add( m_slider8_KC, 0, wxALL, 5 );
	
	bSizer281->Add( bSizer29, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer30;
	bSizer30 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer32;
	bSizer32 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText12 = new wxStaticText( pan_coefficients, wxID_ANY, wxT("                               0.01               1.0"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText12->Wrap( -1 );
	bSizer32->Add( m_staticText12, 1, wxALL|wxEXPAND, 5 );
	
	bSizer30->Add( bSizer32, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer31;
	bSizer31 = new wxBoxSizer( wxHORIZONTAL );
	
	
	m_staticText11 = new wxStaticText( pan_coefficients, wxID_ANY, wxT("deposition constant"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText11->Wrap( -1 );
	bSizer31->Add( m_staticText11, 0, wxALL, 5 );
	
	//deposition constant
	m_slider9 = new wxSlider( pan_coefficients,SLIDER_DEPOSITION_CONSTANT, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	bSizer31->Add( m_slider9, 0, wxALL, 5 );
	
	bSizer30->Add( bSizer31, 1, wxEXPAND, 5 );
	
	bSizer281->Add( bSizer30, 1, wxEXPAND, 5 );
	
	bSizer41->Add( bSizer281, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer33;
	bSizer33 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer34;
	bSizer34 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer35;
	bSizer35 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer38;
	bSizer38 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText15 = new wxStaticText( pan_coefficients, wxID_ANY, wxT("                           0.01                 1.0"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText15->Wrap( -1 );
	bSizer38->Add( m_staticText15, 0, wxALL, 5 );
	
	bSizer35->Add( bSizer38, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer37;
	bSizer37 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText13 = new wxStaticText( pan_coefficients, wxID_ANY, wxT("dissolving constant"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText13->Wrap( -1 );
	bSizer37->Add( m_staticText13, 0, wxALL, 5 );
	
	//dissolving constant
	m_slider10 = new wxSlider( pan_coefficients, SLIDER_DISSOLVING_CONSTANT, 30, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	bSizer37->Add( m_slider10, 0, wxALL, 5 );
	
	bSizer35->Add( bSizer37, 1, wxEXPAND, 5 );
	
	bSizer34->Add( bSizer35, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer36;
	bSizer36 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer40;
	bSizer40 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText16 = new wxStaticText( pan_coefficients, wxID_ANY, wxT("                           0.01                 1.0"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText16->Wrap( -1 );
	bSizer40->Add( m_staticText16, 0, wxALL, 5 );
	
	bSizer36->Add( bSizer40, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer39;
	bSizer39 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText14 = new wxStaticText( pan_coefficients, wxID_ANY, wxT("evaporation const"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText14->Wrap( -1 );
	bSizer39->Add( m_staticText14, 0, wxALL, 5 );
	
	//evaporation constant
	m_slider11 = new wxSlider( pan_coefficients, SLIDER_EVAPORATION_CONSTANT, 50, 20, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	bSizer39->Add( m_slider11, 0, wxALL, 5 );
	
	bSizer36->Add( bSizer39, 1, wxEXPAND, 5 );
	
	bSizer34->Add( bSizer36, 1, wxEXPAND, 5 );
	
	bSizer33->Add( bSizer34, 1, wxEXPAND, 5 );
	
	bSizer41->Add( bSizer33, 1, wxEXPAND, 5 );
	
	pan_coefficients->SetSizer( bSizer41 );
	pan_coefficients->Layout();
	bSizer41->Fit( pan_coefficients );
	m_notebook21->AddPage( pan_coefficients, wxT("coefficients"), false );
	m_panel13 = new wxPanel( m_notebook21, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer53;
	bSizer53 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer55;
	bSizer55 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText28 = new wxStaticText( m_panel13, wxID_ANY, wxT("               0.01                 1.0"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText28->Wrap( -1 );
	bSizer55->Add( m_staticText28, 0, wxALL, 5 );
	
	
	bSizer55->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText27 = new wxStaticText( m_panel13, wxID_ANY, wxT("0.01                  1"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText27->Wrap( -1 );
	bSizer55->Add( m_staticText27, 0, wxALL, 5 );
	
	bSizer53->Add( bSizer55, 0, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer54;
	bSizer54 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText26 = new wxStaticText( m_panel13, wxID_ANY, wxT("Timestep:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText26->Wrap( -1 );
	bSizer54->Add( m_staticText26, 0, wxALL, 5 );
	
	m_slider20 = new wxSlider( m_panel13, SLIDER_TIMESTEP, 10, 1, 1000, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	bSizer54->Add( m_slider20, 0, wxALL, 5 );
	
	
	bSizer54->Add( 0, 0, 1, wxEXPAND, 5 );
	
	m_staticText261 = new wxStaticText( m_panel13, wxID_ANY, wxT("Minimum alpha:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText261->Wrap( -1 );
	m_staticText261->SetToolTip( wxT("The minimum angle between two cells for sediment computation") );
	
	bSizer54->Add( m_staticText261, 0, wxALL, 5 );
	
	m_slider18 = new wxSlider( m_panel13, MINIMUM_ALPHA, 50, 1, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	bSizer54->Add( m_slider18, 0, wxALL, 5 );
	
	bSizer53->Add( bSizer54, 0, wxEXPAND, 5 );
	
	wxStaticBoxSizer* sbSizer2;
	sbSizer2 = new wxStaticBoxSizer( new wxStaticBox( m_panel13, wxID_ANY, wxT("Pipe") ), wxHORIZONTAL );
	
	m_staticText30 = new wxStaticText( m_panel13, wxID_ANY, wxT("length_x:"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText30->Wrap( -1 );
	sbSizer2->Add( m_staticText30, 0, wxALL, 5 );
	
	wxBoxSizer* bSizer541;
	bSizer541 = new wxBoxSizer( wxVERTICAL );
	
	m_staticText32 = new wxStaticText( m_panel13, wxID_ANY, wxT("0.2                  20"), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText32->Wrap( -1 );
	bSizer541->Add( m_staticText32, 0, wxALL, 5 );
	
	m_slider22 = new wxSlider( m_panel13, wxID_ANY, 10, 2, 200, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL );
	bSizer541->Add( m_slider22, 0, wxALL, 5 );
	
	sbSizer2->Add( bSizer541, 1, wxEXPAND, 5 );
	
	wxBoxSizer* bSizer551;
	bSizer551 = new wxBoxSizer( wxHORIZONTAL );
	
	m_staticText31 = new wxStaticText( m_panel13, wxID_ANY, wxT("Iterations: "), wxDefaultPosition, wxDefaultSize, 0 );
	m_staticText31->Wrap( -1 );
	bSizer551->Add( m_staticText31, 0, wxALL, 5 );
	
	m_slider21 = new wxSlider( m_panel13, SLIDER_ITERATIONS, 10, 2, 200, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL|wxSL_LABELS );
	bSizer551->Add( m_slider21, 0, wxALL, 5 );
	
	sbSizer2->Add( bSizer551, 1, wxEXPAND, 5 );
	
	bSizer53->Add( sbSizer2, 1, wxEXPAND, 5 );
	
	m_panel13->SetSizer( bSizer53 );
	m_panel13->Layout();
	bSizer53->Fit( m_panel13 );
	m_notebook21->AddPage( m_panel13, wxT("expert"), false );

 //Visualisation constrolls
	m_panel131 = new wxPanel( m_notebook21, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL );
	wxBoxSizer* bSizer56;
	bSizer56 = new wxBoxSizer( wxVERTICAL );
	
	wxBoxSizer* bSizer57;
	bSizer57 = new wxBoxSizer( wxHORIZONTAL );
	
	m_toggleBtn3 = new wxToggleButton( m_panel131, TOGGLE_SEDIMENT_PREVIEW, wxT("Sediment_preview"), wxDefaultPosition, wxDefaultSize, 0 );
	m_toggleBtn3->SetValue( true ); 
	bSizer57->Add( m_toggleBtn3, 0, wxALL, 5 );
	
	m_toggleBtn4 = new wxToggleButton( m_panel131, TOGGLE_WATER_PREVIEW, wxT("Water_preview"), wxDefaultPosition, wxDefaultSize, 0 );
	m_toggleBtn3->SetValue( true ); 
	bSizer57->Add( m_toggleBtn4, 0, wxALL, 5 );
	
	bSizer56->Add( bSizer57, 1, wxEXPAND, 5 );
	
	m_panel131->SetSizer( bSizer56 );
	m_panel131->Layout();
	bSizer56->Fit( m_panel131 );
	m_notebook21->AddPage( m_panel131, wxT("visualization"), false );
	
	//add water simpanel
	bSizer271->Add( m_notebook21, 1, wxEXPAND | wxALL, 5 );
	
	Water_sim_panel->SetSizer( bSizer271 );
	Water_sim_panel->Layout();
	bSizer271->Fit( Water_sim_panel );
	m_notebook2->AddPage( Water_sim_panel, wxT("Water_sim"), false );


	//#### alles zusammenfuegen (bei Panel 1) ####//	
	
	panelsizer->Add(m_notebook2,1,wxGROW , 20);
	
	// Das OpenGl fenster
	wxBoxSizer *glPlanesizer = new wxBoxSizer(wxHORIZONTAL);
	//glPanel = new wxPanel(this, wxID_ANY,wxDefaultPosition,wxSize(width,500));
	int args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};
	glPlane = new BasicGLPane( (wxFrame*)  m_panel12, args); //das OPENGL Fenster
	glPlanesizer->Add(glPlane,1,  wxGROW,20);
	
	//die beiden GUI berieche in einen sizer zusammenfuegen
	generalsizer->Add(panelsizer,4,wxEXPAND|wxFIXED_MINSIZE |wxALIGN_TOP );
	generalsizer->Add(glPlanesizer,8,wxGROW); //proportion 4:8:0

		
	m_panel12->SetSizer( generalsizer );
	m_panel12->Layout();
	generalsizer->Fit( m_panel12 );

	//### Panel 2 mit dem Auto generiertem Bild #### ///
	//Bitmap_panel ist meine eigene class, abgeleitet von wxpanel
	panel_preview_picture = new Bitmap_Panel( m_splitter3, PANEL_PREVIEW_PICTURE, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL ,"panel");
	wxBoxSizer* bSizer22;
	bSizer22 = new wxBoxSizer( wxVERTICAL );
	
	Pfeil_RU = new wxBitmap(wxT("BITMAP_PFEIL_RU"),wxBITMAP_TYPE_RESOURCE);
	Pfeil_LO = new wxBitmap(wxT("BITMAP_PFEIL_LO"),wxBITMAP_TYPE_RESOURCE);
	open_close_pic_but = new wxBitmapButton(panel_preview_picture, ON_OFF_PIC_BUT,*Pfeil_RU, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	bSizer22->Add( open_close_pic_but, 0, wxALL, 5 );
	
	crosshair_bmp = new wxBitmap( wxT("BITMAP_CROSSHAIR"), wxBITMAP_TYPE_RESOURCE );
	crosshair_bmp_deact = new wxBitmap( wxT("BITMAP_CROSSHAIR_DEACTIVATED"), wxBITMAP_TYPE_RESOURCE );
	m_bitmap_button_setflag = new wxBitmapButton( panel_preview_picture, BUTTON_TOGGLE_CROSSHAIR,*crosshair_bmp , wxPoint( 100,100 ), wxDefaultSize, wxBU_AUTODRAW|wxSUNKEN_BORDER );
	m_bitmap_button_setflag->SetToolTip( wxT("Switch on to set position flag on the preview picture") );
	
	
	bSizer22->Add( m_bitmap_button_setflag, 0, wxALIGN_RIGHT|wxALL, 5 );

	panel_preview_picture->SetSizer( bSizer22 );
	//den Splitter einstellen
	m_splitter3->SplitVertically( m_panel12,panel_preview_picture, 700);
	m_splitter3->SetMinimumPaneSize(55); //verhindert unsplit
	// Dem hoechsten Sier den Splitter uebergeben
	Super_sizer->Add( m_splitter3, 1, wxEXPAND, 5 );



	m_notebook2->SetImageList(imagelist);
	m_notebook2->SetPageImage(0,0);
	m_notebook2->SetPageImage(1,1);
	m_notebook2->SetPageImage(4,2);
	m_notebook2->SetPageImage(2,3);
	m_notebook2->SetPageImage(3,4);
	m_notebook2->SetPageImage(5,5);
	m_notebook2->SetPageImage(6,6);
	//## Icons setzen ## //
	icon.LoadFile(wxT("ICON_TERRAIN"),wxBITMAP_TYPE_ICO_RESOURCE);
	icon_big.LoadFile(wxT("ICON_TERRAIN_BIG"),wxBITMAP_TYPE_ICO_RESOURCE);
	icon_bundle.AddIcon(icon);
	icon_bundle.AddIcon(icon_big);


	

	errorstream.open("error.txt");
	sediment_preview_on = true;
	water_preview_on = true;

	
	
	// Den hoechsten Sizer angeben//
	SetSizer(Super_sizer);
	SetAutoLayout(TRUE);
	Layout();

	


}

BasicFrame::~BasicFrame() {
	delete glPlane;
	//delete preview_bitmap;
	delete crosshair_bmp, delete Pfeil_RU, delete Pfeil_LO, delete crosshair_bmp_deact;
	//delete Save_mask;
	//delete pfeil_mask;
	//delete Open_mask;
	errorstream.close();
	delete [] imagelist;

	
	 
}


void BasicFrame::Open_close_pic(wxCommandEvent &event){
	if (side_window == true) {
		side_window = false;
		m_splitter3->GetWindow2() ->ClearBackground();
		m_splitter3->GetWindow2() ->Update(); //updated und leoscht somit das dc
		//rechtes window auf 70 pix verkleinern (so dass man den button noch sehen kann)
		m_splitter3->SetSashPosition(int((this->GetSize()).GetWidth()) - 70,true);
		open_close_pic_but->SetBitmapLabel(*Pfeil_LO);
	}
	else {
		side_window = true;
		//rechtes window so gross wie das bild + 20 machen
		m_splitter3->SetSashPosition(int((this->GetSize()).GetWidth()) - Alti.getxsize() - 40,true);
		UpdateImage();
		ManualRender();
		open_close_pic_but->SetBitmapLabel(*Pfeil_RU);
		//panel_preview_picture->init_flag();
	}
}

void BasicFrame::UpdateImage(void){
	wxImage temp_pic = Alti.Make_Preview_Image();//gibt preview_image zurueck
	global_preview_bitmap =  wxBitmap(temp_pic,-1);
	
	if ( sediment_preview_on ){
	global_sediment_bitmap = wxBitmap( Alti.Make_Sediment_Image());
	}

	temp_pic.Destroy(); //wichtig, da sonst memory leaks
}


void BasicFrame::ManualRender(){ //rendert nur auf aufruf von "meinen" funktionen (im gegensatz zu auf paint eventen)
	if (side_window == true) {
		m_splitter3->UpdateSize();//sehr wichtig, da sonst bei groessenaendern das bild verkackt
		panel_preview_picture->render();
		//wxClientDC dc( m_splitter3->GetWindow2() );
		//dc.DrawBitmap(*global_preview_bitmap,-10,100,false);
	}
}
void BasicFrame::HandleSash_position(wxSplitterEvent &event){
	ManualRender();
}


void BasicFrame::DrawPoint(wxMouseEvent &event){
		wxPen def_pen = wxPen("wxRED_PEN",1,1);
		wxClientDC dc( m_splitter3->GetWindow2() );
		dc.DrawPoint(event.GetX(),event.GetY());
		m_splitter3->SetSashPosition(int((this->GetSize()).GetWidth()) - 70,true);
		event.Skip();

}

void BasicFrame::ToggleCrosshair(wxCommandEvent &event){
	if (crosshair==true){
		crosshair = false;
		m_bitmap_button_setflag->SetBitmapLabel(*crosshair_bmp_deact);
	}
	else{
		crosshair = true;
		m_bitmap_button_setflag->SetBitmapLabel(*crosshair_bmp);
	}
	panel_preview_picture->ToggleFlag(crosshair);
}

//void BasicFrame::LeftUp(wxMouseEvent& event){
//	panel_preview_picture->Update();
//	glPlane->setFlag(panel_preview_picture->x,panel_preview_picture->y);
//}

void BasicFrame::UpdateChain(void){
		UpdateImage();
		ManualRender();
		glPlane->refresh_terrain();
		glPlane->Refresh(); 
}












void BasicFrame::Onthebutton(wxCommandEvent &event){
	Close();
}
void BasicFrame::Erosion_Button(wxCommandEvent &event){
	Alti.erosion(Erosion_radius,Erosion_iterationen);
	UpdateChain();
}

void BasicFrame::Thermal_Erosion_Button(wxCommandEvent &event){
	double Talus;
	if (toggle_talus_auto == true)
	{
		Talus = 4.0 / Alti.getxsize(); //auto wert
	}
	else
	{
		Talus = Get_Talus_Threshold();
	}
	int iterations = erosion_slider_iterationen->GetValue();
	Alti.ThermalErosion(iterations*10,Talus);
	UpdateChain();
}

void BasicFrame::ToggleAuto(wxCommandEvent &event){
	toggle_talus_auto = (bool)m_toggleBtn5->GetValue();
	if (toggle_talus_auto == true)
	{
		m_textCtrl3->Clear();
		string num = num_to_string( double(4.0/Alti.getxsize()));
		num = num.erase(0,2); // das "0." löschen
		*m_textCtrl3 << num;
		m_textCtrl3->Enable(false);

	}
	else
	{

		m_textCtrl3->Enable(true);
	}
}

double BasicFrame::Get_Talus_Threshold()
{
	wxString inp = m_textCtrl3->GetLineText(0);

	stringstream NumberString;
	NumberString << "0." << inp;   
	string Number = NumberString.str();
	char* str = (char*) Number.data();
	double num = atof( str  );//atof convertiert char* to double
	
	
	return num;
}




void BasicFrame::Plateau_Button(wxCommandEvent &event){
	if (Alti.showflag == true){
		Alti.plateau(Alti.FlagX,Alti.FlagY,Plateau_radius);
		UpdateChain();
		flag_pos_text->SetLabel(wxT(""));
	}
	else {
		flag_pos_text->SetLabel(wxT("<-Du musst den Positionflag Button aktivieren!"));
	}
}

void BasicFrame::Randomize_Button(wxCommandEvent &event){
	Alti.randomize(Randomize_radius);//alternativ : randomize2
	UpdateImage();
	ManualRender();
	glPlane->refresh_terrain();
	glPlane->Refresh(); 
}

void BasicFrame::Subdivide_Button(wxCommandEvent &event){
	Alti.subdivision(Coeff,0.0,0.0,0.0,0.0);
	UpdateImage();
	ManualRender();
	glPlane->refresh_terrain(); //das terrain in die display list schreiben
	glPlane->Refresh(); 
}



void BasicFrame::Save_obj(wxCommandEvent &event){
	//Open file dialog
	wxFileDialog *dlg = new wxFileDialog(
		this, wxT("Speichern der .obj wavefront Datei"),wxT(""),wxT(""),
		wxT("Wavefront Dateien (.obj)|*.obj"),
		wxSAVE | wxCHANGE_DIR | wxOVERWRITE_PROMPT, wxDefaultPosition);
	if (dlg->ShowModal() == wxID_OK) {
		bool res =  Alti.save_obj((char * )dlg ->GetFilename().c_str());
		if (res == false){
			errorstream << "fehler beim speichern der .obj datei " << endl;
		}
	}
}
 void BasicFrame::Save_image(wxCommandEvent &event){
	 
	 wxFileDialog *dlg = new wxFileDialog(
		this, wxT("Als Grafik speichern "),wxT(""),wxT(""),
		wxT("Portable Network Graphic (.png)|*.png | JPEG Image (.jpg) |*.jpg | Bitmap (.bmp) | *.bmp"),
		wxSAVE | wxCHANGE_DIR | wxOVERWRITE_PROMPT, wxDefaultPosition);
	 if (dlg->ShowModal() == wxID_OK) {
		 //#### format extrahieren### ///
		 string format;
		 string wildcard(dlg->GetPath().c_str());
		 string::size_type pos = wildcard.find_last_of(".",wildcard.length());//trennpunkt der format angabe
		 format = wildcard.substr(pos,wildcard.length());
		 if (format.at(format.length()-1) == ' '){//format.length() -1 = last
			 format.erase(format.length()-1,1); //loescht letzes whitespace wenn gefunden
		 }
		

		 bool res = Alti.save_image((char * )dlg ->GetFilename().c_str(),format);
		if (res == false){
			errorstream << "fehler beim speichern der bild datei " << endl;
		}
		}
}







void BasicFrame::Open_image_button(wxCommandEvent &event){
	//Open file dialog
	wxFileDialog *dlg = new wxFileDialog(
		this, wxT("Öfnnen der .tga Image Datei"),wxT(""),wxT(""),
		wxT("Image Dateien (png,jpg,bmp,tga)|*.tga;*.png;*.jpg;*.bmp |Targa Bilder (.tga)|*.tga|PNG Bilder (.png)|*.png|JPEG Bilder (.jpg)|*.jpg|Bitmaps (.bmp)|*.bmp "),
		wxOPEN | wxCHANGE_DIR, wxDefaultPosition);

	if (dlg->ShowModal() == wxID_OK) {
		int stat = Alti.terrainLoadFromImage( (char * )dlg ->GetFilename().c_str(),0); //normals  off
		errorstream << "Loadimage " << stat << dlg ->GetFilename().c_str() << endl;
		if (stat != 0) { //fehler beim laden
			int stat2 = Alti.terrainLoadFromImage( (char * )dlg ->GetFilename().c_str(),0,5); // alternativer Loader
			errorstream << "Loadimage2 " << stat2 << endl;
		}

	}
	dlg->Destroy();
	UpdateChain();
}


void BasicFrame::Open_obj_button(wxCommandEvent &event){
	//Open file dialog
	wxFileDialog *dlg = new wxFileDialog(
		this, wxT("Öffnen der .obj Wavefront 3D Datei"),wxT(""),wxT(""),
		wxT("Wavefront Dateien (.obj)|*.obj|Alle Dateien(*.*)|*.*"),
		wxOPEN | wxCHANGE_DIR, wxDefaultPosition);

	if (dlg->ShowModal() == wxID_OK) {
		int stat = Alti.Load_obj( (char * )dlg ->GetFilename().c_str()); //normals  off
		Alti.FlagX = (int)Alti.getxsize() / 2;
		Alti.FlagY = (int)Alti.getysize() / 2;

	}
	dlg->Destroy();
	UpdateImage();
	ManualRender();
	glPlane->refresh_terrain();
	glPlane->Refresh();
}



void BasicFrame::PositionFlagButton(wxCommandEvent &event){
	if (Alti.showflag == true) {
		positionflag->SetBitmapLabel(positionflagbitmap); //den unselektierten zustand annehmen
		Alti.showflag = false;

	}
	else{
		positionflag->SetBitmapLabel(positionflagbitmap_selected); //den selektierten zustand annehmen
		Alti.showflag = true;
	}
	glPlane->Refresh();
}

//-----------Perlin----------------
void BasicFrame::Slider_Persistence(wxCommandEvent &event){
	Perlin_Persistence = double(m_slider201->GetValue()) / 100.0;
}

void BasicFrame::Slider_Perlin_Zoom(wxCommandEvent &event){
	Perlin_Zoom = double(m_slider211->GetValue()) / 100.0;
}

void BasicFrame::Slider_Octaves(wxSpinEvent  &event){
	Octaves = int(m_spinCtrl1->GetValue());

}
void  BasicFrame::Perlin_Noise(wxCommandEvent &event){
	Alti.Perlin(Perlin_Persistence,Octaves,Perlin_Zoom);
	UpdateImage();
	ManualRender();
	glPlane->refresh_terrain(); //das terrain in die display list schreiben
	glPlane->Refresh(); 
}



//----------Voronoi----------------
void BasicFrame::Voronoi(double inp_map[]) //seperate funktion fuer externen zugriff
{
		wxString clip_num = m_textCtrl1->GetLineText(0);
	clip_num.Replace(wxT("."),wxT(""),true);
	clip_num.Replace(wxT("e"),wxT(""),true);
	clip_num.Replace(wxT("-"),wxT(""),true);
	clip_num.Replace(wxT("+"),wxT(""),true);
	clip_num.insert(0,wxT("0."));
	double clip=0.0;
	clip_num.ToDouble(&clip);

	int blockSize = int(m_slider_blocksize->GetValue());
	int intesity = int(m_slider_fValue->GetValue());
	bool negative = m_checkBox_negative->GetValue();
	int distance = m_choice_distance->GetCurrentSelection()+1;//+1, da distance_methods nummer 1-5 haben

	wxString coeffs_str = m_textCtrl2_vor_coeff->GetLineText(0);
	if (coeffs_str.size() != 0)
	{

		coeffs_str+= ";"; //damit auch das letze wort hinzugefuegt wird
		wxString word = wxT("");
		float  tmp[20];
		double coeff;
		int words = 0;

		for(size_t i = 0; i < coeffs_str.size();i++){
			if(coeffs_str[i] != ';')
			{
				word += coeffs_str[i];
			}
			else
			{
				
				word.ToDouble(&coeff);
				tmp[words] =float(coeff);
				word = wxT("");
				words++;
			}
		}
		if (inp_map == NULL){
			Alti.Voronoi_Tex(intesity,blockSize,negative,float(clip),distance,tmp,words);
		}
		else
		{
			Alti.Inp_Voronoi_Tex(intesity,blockSize,negative,float(clip),distance,inp_map,tmp,words);
		}

	}

	else{
		if (inp_map == NULL){
			Alti.Voronoi_Tex(intesity,blockSize,negative,float(clip),distance);
		}
		else{
			Alti.Inp_Voronoi_Tex(intesity,blockSize,negative,float(clip),distance,inp_map);
		}

	}


}
void BasicFrame::Button_Voronoi(wxCommandEvent &event){
	Voronoi();
	
	UpdateImage();
	ManualRender();
	glPlane->refresh_terrain(); //das terrain in die display list schreiben
	glPlane->Refresh(); 
}

//-----------------------Combined-----------------------------
void BasicFrame::Spinctrl1(wxSpinEvent &event){
	factor_combination1 = m_spinCtrl2_add_factor_1->GetValue();
	if (factor_combination1 == 0)
	{
		factor_combination1 = 1;
		m_spinCtrl2_add_factor_1->SetValue(1); //erste listbox darf nie 0 werden, da sons keine operation ausgeführt wird
	}
}

void BasicFrame::Spinctrl2(wxSpinEvent &event){
	factor_combination2 = m_spinCtrl2_add_factor_2->GetValue();
}

void BasicFrame::Spinctrl3(wxSpinEvent &event){
	factor_combination3 = m_spinCtrl2_add_factor_3->GetValue();
}

void BasicFrame::Choice_combined1(wxCommandEvent &event){
	Choice_combination1 = m_choice21->GetCurrentSelection();
}
void BasicFrame::Choice_combined2(wxCommandEvent &event){
	Choice_combination2 = m_choice2->GetCurrentSelection();
	if (Choice_combination2 == 3) // 3 = None
	{
		factor_combination2 = 0;
		m_spinCtrl2_add_factor_2->SetValue(0);
	}

}
void BasicFrame::Choice_combined3(wxCommandEvent &event){
	Choice_combination3 = m_choice22->GetCurrentSelection();
	if (Choice_combination3 == 3) // 3 = None
	{
		factor_combination3 = 0;
		m_spinCtrl2_add_factor_3->SetValue(0);
	}
	else if (Choice_combination3 != 3 && factor_combination3==0){
		factor_combination3 = 1;
	}
}


bool BasicFrame::Execute_Choice(int choice,double inp_map[])
{
	if (choice != 3) // 3 = None
	{
		if (choice == 0) //0 = Midpoint Displacement
		{
			Alti.Inp_subdivision(Coeff,inp_map);
		}
		else if (choice == 1) //1 = Perlin
		{
			Perlin_Persistence = double(m_slider201->GetValue()) / 100.0;
			Perlin_Zoom = double(m_slider211->GetValue()) / 100.0;
			Octaves = int(m_spinCtrl1->GetValue());
			Alti.Inp_Perlin(inp_map,Perlin_Persistence,Octaves,Perlin_Zoom);
		}
		else if (choice==2) // 2 = Voronoi
		{
			this->Voronoi(inp_map);
		}
		return true;
	}
	else {
		return false;}//keine operation
}





void BasicFrame::Combined_Generate(wxCommandEvent &event){
	int xsize = Alti.getxsize();
	int ysize = Alti.getysize();
	Copy_map1 = new double [xsize * ysize +1];
	Copy_map2 = new double [xsize * ysize +1];
	Copy_map3 = new double [xsize * ysize +1];
	Execute_Choice(Choice_combination1,Copy_map1);
	bool two = Execute_Choice(Choice_combination2,Copy_map2);
	bool three = Execute_Choice(Choice_combination3,Copy_map3);
	int whole = factor_combination1+ factor_combination2+factor_combination3;
	float fac_one = factor_combination1/float(whole);
	float fac_two = factor_combination2/float(whole);
	float fac_three = factor_combination3/float(whole);
	double val;
	

	for ( int x = 0;x <xsize; ++x){
		for (int y=0; y < ysize;++y){
			Alti.Alt(x,y,Copy_map1[x*ysize+y]*fac_one);
			if (two)
			{
				val = (Copy_map2[x*ysize+y])*fac_two;
				Alti.add(x,y,val);//-0.5, damit addiert und abgezogen wird
			}
			if (three)
			{
				Alti.multiply(x,y,Copy_map3[x*ysize+y]*fac_three);
			}
		}
	}


	delete [] Copy_map1;
	delete [] Copy_map2;
	delete [] Copy_map3;
	
	UpdateImage();
	ManualRender();
	glPlane->refresh_terrain(); //das terrain in die display list schreiben
	glPlane->Refresh(); 
}

void BasicFrame::Button_Save_Preset(wxCommandEvent &event){
	wxFileDialog *dlg = new wxFileDialog(
		this, wxT("Preset Speichern "),wxT(""),wxT(""),
		wxT("Text (.txt) | *.txt"),
		wxSAVE | wxCHANGE_DIR | wxOVERWRITE_PROMPT, wxDefaultPosition);
	 if (dlg->ShowModal() == wxID_OK) {

		 string path(dlg->GetPath().c_str());
		 Save_Preset_Text(path);
	 }
}


void BasicFrame::Button_Load_Preset(wxCommandEvent &event){
	wxFileDialog *dlg = new wxFileDialog(
		this, wxT("Preset Laden"),wxT(""),wxT(""),
		wxT("Text (.txt) | *.txt"),
		wxOPEN | wxCHANGE_DIR, wxDefaultPosition);
	if (dlg->ShowModal() == wxID_OK) {
		string path(dlg->GetPath().c_str());
		Load_Preset_Text(path);
	}
	dlg->Destroy();
}


void BasicFrame::Save_Preset_Text(string path)
{
	save_preset_stream.open(path.c_str());
	if (save_preset_stream.fail() == true)
	{
		Error_dialog(std::string("Error creating File"));
	}
	save_preset_stream << "#Terrain Generator Preset File";
	save_preset_stream << Get_Values(m_choice21->GetCurrentSelection() );
	save_preset_stream << "Fac:" << num_to_string(factor_combination1);
	
	save_preset_stream << Get_Values(m_choice2->GetCurrentSelection() );
	save_preset_stream << "Fac:" << num_to_string(factor_combination2);
	
	save_preset_stream << Get_Values(m_choice22->GetCurrentSelection() );
	save_preset_stream << "Fac:" << num_to_string(factor_combination3);
	
	save_preset_stream.close();


}
void BasicFrame::Load_Preset_Text(string path)
{
	char buffer[512];
	read_preset_stream.open(path.c_str());
	
	read_preset_stream.seekg(0,ios::end);//cursor ans ende
	int endpos = read_preset_stream.tellg();

	read_preset_stream.seekg(0,ios::beg); //cursor wieder auf 0
	if (read_preset_stream.fail() == true)
	{
		Error_dialog(string("Error reading File"));
		read_preset_stream.close();
		return;
	}
	string first = Readline(read_preset_stream,buffer);
	if ( first.compare("#Terrain Generator Preset File") != 0 )
	{
		Error_dialog(string("Not a preset File"));
		read_preset_stream.close();
		return;
	}
	
	int num  = 0; //welche der drei wxChoices
	while ( int(read_preset_stream.tellg()) != endpos)
	{
		string line = Readline(read_preset_stream,buffer);
		
		if (line.empty() == true)
		{
			continue;
		}
		string::size_type tp = line.find("#");//bezeichnet neue noise method
		if (tp != string::npos)//wenn noise method gefunden
		{
			string Noise_method = line.substr(tp+1);
			if (Noise_method.compare("Midpoint") == 0)
			{
				Change_wxChoice(0,num);//0=midpoint
			}
			else if (Noise_method.compare("Perlin") == 0)
			{
				Change_wxChoice(1,num);//1=Perlin
			}
			else if (Noise_method.compare("Voronoi") == 0)
			{
				Change_wxChoice(2,num);//2=Voronoi
			}
			else if (Noise_method.compare("None") == 0)
			{
				Change_wxChoice(3,num);//3=None
			}
			else
			{
				Error_dialog(string("Error in Preset File"));
			}
			num++;
		}
		else if (line.find(":") != string::npos)
		{
			string Prop = line.substr(0, line.find(":") );
			string Value = line.substr(line.find(":")+1);
			int type = 1;//type argument, um der string to num funktion den geweünschten ausgabewert zu zeigen
			float ftype = 1.0;
			int iValue = string_to_num(Value,type); //
			float flValue = string_to_num(Value,ftype);
			//Error_dialog(Prop+"|" +Value);
			if ( Prop.find("Persistence") != string::npos)
			{
				m_slider201->SetValue( int(flValue * 100.0) );
				Perlin_Persistence = double(m_slider201->GetValue()) / 100.0;
			}
			else if (Prop.find("Zoom") != string::npos)
			{
				m_slider211->SetValue( int(flValue * 100.0) );
				Perlin_Zoom = double(m_slider211->GetValue()) / 100.0;
			}
			else if (Prop.find("Octaves") != string::npos)
			{
				m_spinCtrl1->SetValue(iValue);
				Octaves = int(m_spinCtrl1->GetValue());
			}
			else if (Prop.find("Fac") != string::npos)
			{
				switch (num)
				{
				case 1://num ist schon min 1 einmal erhöht worden
					m_spinCtrl2_add_factor_1->SetValue(iValue);
					factor_combination1 = iValue;
					break;
				case 2:
					m_spinCtrl2_add_factor_2->SetValue(iValue);
					factor_combination2 = iValue;
					break;
				case 3:
					m_spinCtrl2_add_factor_3->SetValue(iValue);
					factor_combination3 = iValue;
					break;
				}
			}
			else if (Prop.find("BlockSize") != string::npos)
			{
				m_slider_blocksize->SetValue(iValue);
			}
			else  if (Prop.find("Intesity") != string::npos)
			{
				m_slider_fValue->SetValue(iValue);
			}
			else if (Prop.find("Negative") != string::npos)
			{
				m_checkBox_negative->SetValue( iValue );
			}
			else if (Prop.find("Distance") != string::npos)
			{
				m_choice_distance->SetSelection(iValue);
			}
			else if (Prop.find("Coefficients") != string::npos)
			{
				m_textCtrl2_vor_coeff->Clear();//remove current selection
				m_textCtrl2_vor_coeff->AppendText(Value);
			}
			else if (Prop.find("Clip") != string::npos)
			{
				m_textCtrl1->Clear();
				m_textCtrl1->AppendText(Value);
			}
			else if (Prop.find("Coeff") != string::npos)
			{
				subdiv_slider_coeff->SetValue(iValue);
				Coeff = double( subdiv_slider_coeff->GetValue()) / float(coeff_sizer);// + (double(slider_fine_subdivision->GetValue())   / 100.0); //herunterskalieren, slider nimmt nur int
			}
		}
	}

	read_preset_stream.close();
}


void BasicFrame::Change_wxChoice(int choice,int num /* welche der drei choiceboxes*/)
		{
			//wxString m_choice21Choices[] = { wxT("Midpoint Displacement"), wxT("Perlin"), wxT("Voronoi") };
			//int m_choice21NChoices = sizeof( m_choice21Choices ) / sizeof( wxString );

			switch (num)
			{
			case 0:
				m_choice21->SetSelection( choice );
				Choice_combination1 = choice;
				break;
			case 1:
				m_choice2->SetSelection( choice );
				Choice_combination2 = choice;
				break;
			case 2:
				m_choice22->SetSelection( choice );
				Choice_combination3 = choice;
				break;
			}
		}




string  BasicFrame::Readline(std::ifstream& file,char buffer[])
{
	file.getline(buffer,512 );
	stringstream Line;
	Line << buffer;
	return Line.str();
}



template <class T>
T BasicFrame::string_to_num(string input,T wished_type)
{
	double num = atof( input.c_str() );//atof convertiert char* to double
	T out = static_cast<T>(num);
	return out;
}







template <class T>
string BasicFrame::num_to_string(T input)
{
	stringstream NumberString;
	NumberString << input;   
	string Number = NumberString.str();
	return Number;
}

string BasicFrame::Get_Values(int choice)
{
	string out("\n#");
	if (choice == 0) //0 = Midpoint Displacement
		{
			out.append("Midpoint");
			out.append("\n");
			out.append("Coeff:" ).append(num_to_string(Coeff)).append("\n");
		}
		else if (choice == 1) //1 = Perlin
		{
			Perlin_Persistence = double(m_slider201->GetValue()) / 100.0;
			Perlin_Zoom = double(m_slider211->GetValue()) / 100.0;
			Octaves = int(m_spinCtrl1->GetValue());
			
			out.append("Perlin").append("\n");
			out.append("Persistence:").append( num_to_string(Perlin_Persistence) ).append("\n");
			out.append("Zoom:").append( num_to_string(Perlin_Zoom) ).append("\n");
			out.append("Octaves:").append( num_to_string(Octaves) ).append("\n");
		}
		else if (choice==2) // 2 = Voronoi
		{
			int blockSize = int(m_slider_blocksize->GetValue());
			int intesity = int(m_slider_fValue->GetValue());
			bool negative = m_checkBox_negative->GetValue();
			int distance = m_choice_distance->GetCurrentSelection()+1;//+1, da distance_methods nummer 1-5 haben
			wxString coeffs_str = m_textCtrl2_vor_coeff->GetLineText(0);
			wxString clip_num = m_textCtrl1->GetLineText(0);
			
			out.append("Voronoi").append("\n");
			out.append("BlockSize:").append(  num_to_string(blockSize)  ).append("\n");
			out.append("Intesity:").append(  num_to_string(intesity)  ).append("\n");
			out.append("Negative: ").append(  num_to_string(int(negative))  ).append("\n");
			out.append("Distance:").append(  num_to_string(distance-1)  ).append("\n");
			out.append("Coefficients:").append( coeffs_str.c_str() ).append("\n");
			out.append("Clip:").append(  clip_num.c_str()  ).append("\n");
		}
		else if (choice == 3)
		{
			out.append("None").append("\n");
		}
		return out;

}


void BasicFrame::Perturbation_Button(wxCommandEvent &event){
	Perturbation perturb = Perturbation(0.15);
	if (m_radioBtn1_midpoint_displ->GetValue() == true)
	{
		perturb.Apply(0); // 0 = midpoint displacement
	}
	else if (m_radioBtn2->GetValue() == true)
	{
		perturb.Apply(1); // 1 = Perlin
	}
	UpdateImage();
	ManualRender();
	glPlane->refresh_terrain(); //das terrain in die display list schreiben
	glPlane->Refresh(); 



}




//#----------------------------------------------------------------------------






	
void BasicFrame::Error_dialog(string text){
		wxMessageDialog *error = new wxMessageDialog(this,text.c_str(),wxT("Error"),wxOK | wxICON_ERROR);
		error->ShowModal();
		error->Destroy();
}





void BasicFrame::Slider_Subdivision(wxCommandEvent &event){
	Coeff = double( subdiv_slider_coeff->GetValue()) / float(coeff_sizer);// + (double(slider_fine_subdivision->GetValue())   / 100.0); //herunterskalieren, slider nimmt nur int
}

void BasicFrame::Slider_Erosion_radius(wxCommandEvent &event){
	Erosion_radius = (int) erosion_slider_radius->GetValue();
}
void BasicFrame::Slider_Erosion_iterationen(wxCommandEvent &event){
	Erosion_iterationen = (int) erosion_slider_iterationen->GetValue();
}
void BasicFrame::Randomize_Slider(wxCommandEvent &event){
	Randomize_radius = static_cast<double>(random_fac_slider->GetValue()) ;
}

void BasicFrame::Slider_Scale_Terrain(wxCommandEvent &event){
	glPlane->Terrain_height_scale_fac = static_cast<double>(scale_image_terrain->GetValue()); 
	glPlane->refresh_terrain();


	glPlane->Refresh();
}

void BasicFrame::Slider_Plateau_Radius(wxCommandEvent &event){
	Plateau_radius = (int) m_slider6->GetValue(); //m_slider6 = slider plateau
}

void BasicFrame::Scale_Terrain_Real_Button(wxCommandEvent &event){
	int value = m_scale_terrain->GetValue();
	double realfactor;
	if (value == 0)
			realfactor = 0.25;
	else if (value == 1)
			realfactor  = 0.5;
	else if (value == 2)
			realfactor  = 0.75;
	else if (value == 3)
			realfactor = 1.0;
	else if (value == 4)
			realfactor = 1.5;
	else if (value == 5)
			realfactor = 2.0;
	else if (value == 6)
			realfactor = 3.0;
	else if (value == 7){
			realfactor = 4.0; }

	if (realfactor == 1.0) { return; }//bei 1 gibt es keine groessenaenderung
	Alti.Scale_Terrain(realfactor);
	
	/* angezeigte groesse aendern*/
	wxString str1;
	str1.Printf(wxT("Size: %d x %d"), Alti.getxsize(),  Alti.getxsize());
	m_staticText_scale->SetLabel(str1);
	UpdateChain();
}


void BasicFrame::Simulate_Water(wxCommandEvent &event){
	int sel = (int) m_radioBox1_water_methods->GetSelection();	
	glPlane->Start_WaterSimulation(sel+1); //methods sind 1-4
	
	UpdateImage();
	ManualRender();
	glPlane->refresh_terrain();
	glPlane->Refresh();
}



void BasicFrame::Toggle_Watersource(wxCommandEvent &event){
	if (m_toggleBtn2->GetValue() == false){
		m_staticText18->Enable( false );
		m_slider13->Enable( false );
		m_staticText20->Enable( false );
		m_slider14->Enable( false );

		Water_Kcs.Watersource_on = false;
	}
	else{
		m_staticText18->Enable( true );
		m_slider13->Enable( true );
		m_staticText20->Enable(true);
		m_slider14->Enable(true);

		Water_Kcs.Watersource_on = true;
	}
}

void BasicFrame::Toggle_Rain(wxCommandEvent &event){
	if (m_togglerain->GetValue() == false){
		m_staticText17->Enable( false );
		//m_staticText17->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVECAPTIONTEXT ) );
		m_togglerain->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVEBORDER ) );
		m_slider12->Enable( false );
		m_slider12->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_INACTIVECAPTIONTEXT ) );
		Water_Kcs.Rain_on = false;
	}
	else{
		m_staticText17->Enable( true );
		//m_staticText17->SetForegroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT ) );
		//m_staticText17->SetBackgroundColour( wxSystemSettings::GetColour( wxSYS_COLOUR_ACTIVEBORDER ) );
		m_slider12->Enable( true );
		Water_Kcs.Rain_on = true;
	}
}

void BasicFrame::Toggle_Sediment_Preview(wxCommandEvent &event){
	if (m_toggleBtn3->GetValue() == false){
		sediment_preview_on = false; 
		ManualRender();
	}
	else {
		sediment_preview_on = true;
		ManualRender();
	}
}

void BasicFrame::Toggle_Water_Preview(wxCommandEvent &event){
	if (m_toggleBtn4->GetValue() == false){
		water_preview_on = false; 
		ManualRender();
	}
	else {
		water_preview_on = true;
		ManualRender();
	}
}

void BasicFrame::Slider_Rain_Amount(wxCommandEvent &event){
	Water_Kcs.KR = float(m_slider12 ->GetValue())/10000.0;
}

void BasicFrame::Slider_Water_Radius(wxCommandEvent &event){
	Water_Kcs.Radius = float(m_slider14 ->GetValue());
}

void BasicFrame::Slider_Water_Amount(wxCommandEvent &event){
	Water_Kcs.Water_Amount = float(m_slider13 ->GetValue());
}
void BasicFrame::Slider_Sediment_Capacity(wxCommandEvent &event){
	Water_Kcs.KC = float(m_slider8_KC ->GetValue())/1000.0;
}

void BasicFrame::Slider_Deposition_Constant(wxCommandEvent &event){
	Water_Kcs.KD = float(m_slider9 ->GetValue())/100.0;
}
void BasicFrame::Slider_Dissolving_Constant(wxCommandEvent &event){
	Water_Kcs.KS = float(m_slider10 ->GetValue())/100.0;
}
void BasicFrame::Slider_Evaporation_Constant(wxCommandEvent &event){
	Water_Kcs.KE = float(m_slider11 ->GetValue())/100.0;
}
void BasicFrame::Slider_Timestep(wxCommandEvent &event){
	Water_Kcs.DT = float(m_slider20 ->GetValue())/1000.0;
}
void BasicFrame::Slider_Minimum_Alpha(wxCommandEvent &event){
	Water_Kcs.Min_alpha = float(m_slider18 ->GetValue())/100.0;
}
void BasicFrame::Slider_Iterations(wxCommandEvent &event){
	Water_Kcs.Iterations = int(m_slider21 ->GetValue());
}

//events fuer basic frame
BEGIN_EVENT_TABLE(BasicFrame,wxFrame)
EVT_BUTTON(BUTTON_SUBDIVIDE,BasicFrame::Subdivide_Button)
EVT_BUTTON(BUTTON_OPEN_IMAGE,BasicFrame::Open_image_button)
EVT_SLIDER(SLIDER_SUBDIVISION_COEFF,BasicFrame::Slider_Subdivision)
EVT_BUTTON(BUTTON_EROSION,BasicFrame::Erosion_Button)
EVT_BUTTON(BUTTON_RANDOMIZE,BasicFrame::Randomize_Button)
EVT_BUTTON(BUTTON_PLATEAU,BasicFrame::Plateau_Button)
EVT_BUTTON(BUTTON_OPEN_OBJ,BasicFrame::Open_obj_button)
EVT_BUTTON(BITMAP_BUTTON_FLAGPOS,BasicFrame::PositionFlagButton)
EVT_BUTTON(SAVE_OBJ,BasicFrame::Save_obj)
EVT_BUTTON(SAVE_IMAGE,BasicFrame::Save_image)
EVT_SLIDER(RANDOMIZE_SLIDER,BasicFrame::Randomize_Slider)
EVT_SLIDER(SLIDER_SCALE_IMAGE_TERRAIN,BasicFrame::Slider_Scale_Terrain)
EVT_SLIDER(PLATEAU_RADIUS_SLIDER,BasicFrame::Slider_Plateau_Radius)
EVT_SLIDER(SLIDER_PERLIN_ZOOM,BasicFrame::Slider_Perlin_Zoom)
EVT_SLIDER(SLIDER_PERSISTENCE,BasicFrame::Slider_Persistence)
EVT_SPINCTRL(SLIDER_OCTAVES,BasicFrame::Slider_Octaves)
EVT_BUTTON(ON_OFF_PIC_BUT,BasicFrame::Open_close_pic)
//EVT_SPLITTER_SASH_POS_CHANGING(SPLITTER_WINDOW,BasicFrame::HandleSash_position)
EVT_SPLITTER_SASH_POS_CHANGED(SPLITTER_WINDOW,BasicFrame::HandleSash_position)
EVT_SPLITTER_DCLICK(SPLITTER_WINDOW,BasicFrame::HandleSash_position)
EVT_BUTTON(BUTTON_TOGGLE_CROSSHAIR,BasicFrame::ToggleCrosshair)
EVT_BUTTON(BUTTON_SCALE_TERRAIN_REAL,BasicFrame::Scale_Terrain_Real_Button)
EVT_BUTTON(BUTTON_GENERATE,BasicFrame::Perlin_Noise)
EVT_BUTTON(BUTTON_VORONOI,BasicFrame::Button_Voronoi)
EVT_BUTTON(COMBINED_GENERATE,BasicFrame::Combined_Generate)
EVT_SPINCTRL(SPINCTRL1,BasicFrame::Spinctrl1)
EVT_SPINCTRL(SPINCTRL2,BasicFrame::Spinctrl2)
EVT_SPINCTRL(SPINCTRL3,BasicFrame::Spinctrl3)
EVT_CHOICE(CHOICEBOX1,BasicFrame::Choice_combined1)
EVT_CHOICE(CHOICEBOX2,BasicFrame::Choice_combined2)
EVT_CHOICE(CHOICEBOX3,BasicFrame::Choice_combined3)
EVT_BUTTON(SAVE_BUT_PRESET,BasicFrame::Button_Save_Preset)
EVT_BUTTON(OPEN_BUT_PRESET,BasicFrame::Button_Load_Preset)
EVT_BUTTON(BUTTON_PERTURB,BasicFrame::Perturbation_Button)
EVT_BUTTON(BUTTON_THERMAL_EROSION,BasicFrame::Thermal_Erosion_Button)
EVT_TOGGLEBUTTON(TOGGLE_AUTO,BasicFrame::ToggleAuto)



EVT_BUTTON(BUTTON_WATER_SIMULATE,BasicFrame::Simulate_Water)
EVT_TOGGLEBUTTON(TOGGLE_WATERSOURCE,BasicFrame::Toggle_Watersource)
EVT_TOGGLEBUTTON(TOGGLE_RAIN,BasicFrame::Toggle_Rain)
EVT_TOGGLEBUTTON(TOGGLE_SEDIMENT_PREVIEW,BasicFrame::Toggle_Sediment_Preview)
EVT_TOGGLEBUTTON(TOGGLE_WATER_PREVIEW,BasicFrame::Toggle_Water_Preview)
EVT_SLIDER(SLIDER_RAIN_WATER_AMOUNT,BasicFrame::Slider_Rain_Amount)
EVT_SLIDER(SLIDER_WATER_RADIUS,BasicFrame::Slider_Water_Radius)
EVT_SLIDER(SLIDER_WATER_AMOUNT,BasicFrame::Slider_Water_Amount)
EVT_SLIDER(SLIDER_SEDIMENT_CAPACITY,BasicFrame::Slider_Sediment_Capacity)
EVT_SLIDER(SLIDER_DEPOSITION_CONSTANT,BasicFrame::Slider_Deposition_Constant)
EVT_SLIDER(SLIDER_DISSOLVING_CONSTANT,BasicFrame::Slider_Dissolving_Constant)
EVT_SLIDER(SLIDER_EVAPORATION_CONSTANT,BasicFrame::Slider_Evaporation_Constant)
EVT_SLIDER(SLIDER_TIMESTEP,BasicFrame::Slider_Timestep)
EVT_SLIDER(MINIMUM_ALPHA,BasicFrame::Slider_Minimum_Alpha)
EVT_SLIDER(SLIDER_ITERATIONS,BasicFrame::Slider_Iterations)
END_EVENT_TABLE()



//event table fuer gl events
BEGIN_EVENT_TABLE(BasicGLPane, wxGLCanvas)
EVT_MOTION(BasicGLPane::mouseMoved)
//EVT_LEFT_DOWN(BasicGLPane::mouseDown)
EVT_LEFT_UP(BasicGLPane::mouseReleased)
EVT_RIGHT_DOWN(BasicGLPane::rightClick)
EVT_LEAVE_WINDOW(BasicGLPane::mouseLeftWindow)
EVT_SIZE(BasicGLPane::resized)
EVT_KEY_DOWN(BasicGLPane::keyPressed)
EVT_KEY_UP(BasicGLPane::keyReleased)
EVT_MOUSEWHEEL(BasicGLPane::mouseWheelMoved)
EVT_MIDDLE_DOWN(BasicGLPane::middleMouseDown)
EVT_PAINT(BasicGLPane::render)
END_EVENT_TABLE()


void BasicGLPane::mouseMoved(wxMouseEvent& event) {
	if ( event.m_middleDown == true ) //wenn gleichzeitg die mittelmaustaste gedrueckt
	{
		if (event.m_shiftDown == true) { //modifier shift zum translieren
			mouseshift(event.GetX(),event.GetY());
		}
		else { 
			mouseMovement(event.GetX(),event.GetY());
		}
		
	rotate_icon->setHotspot(event.GetX()*0.01,event.GetY()*0.01);
	Refresh();

	
	
	
	}
	if (Alti.showflag==true && event.m_leftDown &&  event.m_middleDown == false && event.m_shiftDown == false) {
		int mousex = event.GetX();
		int mousey = getHeight() - event.GetY();
	
		//gluUnProject(mousex,mousey,0,
		//errorstream << mousex << "," <<  mousey << std::endl;
		if (unproject(mousex,mousey,-5) == GL_FALSE) {return; } //error
		int flagx = static_cast<int>(floor(obj_cox[0]/ terrainStepWidth));
		int flagy = static_cast<int>(floor(obj_coy[0]/ terrainStepLength));
		//errorstream << mousex << "," <<  mousey << ";" << flagx << "," << flagy << std::endl;
		setFlag(abs(flagx),abs(flagy));
	}

}
GLint BasicGLPane::project(int x , int y ,int z ){ 
	//ergebnisse werden in win_co[] geschrieben
	GLdouble model_mat [16];
	GLdouble project_mat [16] ;
	GLint viewp_mat [4];
	
	glGetDoublev(GL_MODELVIEW_MATRIX,model_mat); //werte werden in zweiten parameter geschrieben
	glGetDoublev(GL_PROJECTION_MATRIX,project_mat);
	glGetIntegerv(GL_VIEWPORT,viewp_mat);

	GLint result = gluProject(-x,-y,z,model_mat,project_mat, viewp_mat ,
	win_cox, win_coy ,win_coz);
	return result;
}

GLint  BasicGLPane::unproject(int winx , int winy ,int winz ){
	//ergebnisse werden in obj_co[] geschrieben
	GLdouble model_mat [16];
	GLdouble project_mat [16] ;
	GLint viewp_mat [4];
	
	glPushMatrix(); // die flag auf den stack legen
	defineflag(); //rendert positionsanzeiger

	float ratio_w_h = (float)(getWidth())/(float)( getHeight());
    gluPerspective(45 /*view angle*/, ratio_w_h, 0.1 /*clip close*/, 200 /*clip far*/);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
	glGetDoublev(GL_MODELVIEW_MATRIX,model_mat); //werte werden in zweiten parameter geschrieben

	glViewport(0, 0, getWidth(), getHeight());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

	glTranslatef(xshift,yshift,-5);
	//glRotatef(yrot,0, 1, 0); //xrotation
	//glRotatef(xrot,1, 0, 0); // yrotation

	glGetDoublev(GL_PROJECTION_MATRIX,project_mat);



	glGetIntegerv(GL_VIEWPORT,viewp_mat);


	GLint result = gluUnProject(winx,winy,winz,model_mat,project_mat, viewp_mat ,
	obj_cox,obj_coy,obj_coz);
	return result;
}

//void BasicGLPane::mouseDown(wxMouseEvent& event) {  }

void BasicGLPane::mouseWheelMoved(wxMouseEvent& event) {
	//if (event.m_wheelRotation != 0){
	int fac = event.GetWheelRotation();
	zoom += fac / 120.0;
	Refresh();
	//zoom_func();
	//}
}


void BasicGLPane::rightClick(wxMouseEvent& event) {}
void BasicGLPane::mouseLeftWindow(wxMouseEvent& event) {}
void BasicGLPane::keyPressed(wxKeyEvent& event) {}
void BasicGLPane::keyReleased(wxKeyEvent& event) {}

void BasicGLPane::middleMouseDown(wxMouseEvent& event) {
	lastx = event.GetX(); // verhindert, dass model bei zweiten klcik verspringt
	lasty = event.GetY();
	shiftlastx = event.GetX();
	shiftlasty = event.GetY();
	rotating = true;
	//	if (rotating){
	//wxClientDC dc(this);
	//dc.DrawBitmap(*rotate_bmp,lastx- int(rotate_bmp->GetWidth() / 2.0), lasty- int(rotate_bmp->GetHeight() / 2.0),true);
	//}
}

void BasicGLPane::mouseReleased(wxMouseEvent& event) {
	//wenn kein modifier gedrueckt ist, und flagmode = true
	
	if (event.m_middleDown == false) {
		rotating = false;
	}
}









BasicGLPane::BasicGLPane(wxFrame* parent, int* args) :
wxGLCanvas(parent, wxID_ANY,  wxDefaultPosition, wxDefaultSize, 0, wxT("GLCanvas"),  args) // size altranativ : wxSize(parent->GetMaxWidth(),600)
{
    int argc = 1;
    char* argv[1] = { wxString((wxTheApp->argv)[0]).char_str() };
	alpha = 0;
	xpos = 0, ypos = 0, zpos = 0 , xrot = 0 , yrot = 0, angle = 0.0 ;
	xshift = 0, yshift = 0;
	Terrain_height_scale_fac = 2.0;
	errorstream.open("gl_error.txt");
	g_lightDir[0] = 0.5f, g_lightDir[2] = 0.0f, g_lightDir[3] = 1.0f, g_lightDir[4] = 1.0f;
	light_intesity[0] = 1.0f, light_intesity[1] = 1.0f, light_intesity[2] = 1.0f, light_intesity[3] = 1.0f;
	yellowAmbientDiffuse[0] = 1.0f, yellowAmbientDiffuse[1] = 1.0f, yellowAmbientDiffuse[2] =  1.0f, yellowAmbientDiffuse[3] = 1.0f;
	
	
	m_vertexBuffer = 0;
    m_indexBuffer = 0;
    //m_totalVertices = Alti.getxsize() * Alti.getysize();
    m_totalIndices = 0;
	Update = 0;
	
	/*
	for ( int i=0; i<2500;i++) //initaliesierung von hoehenmap mit defaultwerten
	{
		float ran = ( rand() % 11 ) / 10.0 ; //zahl zwischen 0 und 1
		hoehenmap[i] = ran;
	}*/
	glutInit(&argc, argv);
	//terrainCreate();
	//terrainDL_id =  Maketerrain(); 

	/*
	m_totalIndices = (Alti.getxsize() - 1) * (Alti.getxsize() * 2 + 1);
    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);*/
	
	//win_co = new GLdouble [3]; //werte von gluProject werden als GLdouble*  gespeichert
	//obj_co = new GLdouble [3];
	//win_co_array {double win_co[3]; }; //ein struct array, um array aus einer funtion zurueckzugeben 
	obj_cox  = new GLdouble [2];
	obj_coy  = new GLdouble [2];
	obj_coz  = new GLdouble [2];
	win_cox  = new GLdouble [2];
	win_coy  = new GLdouble [2];
	win_coz  = new GLdouble [2];

	/*Das Png Image als header importieren und in stream leiten*/
	wxMemoryInputStream istream(rotate_icon_png, sizeof(rotate_icon_png));
	wxImage im_rotate_png(istream, wxBITMAP_TYPE_PNG);
	rotating_image = new Image(im_rotate_png); 

	//rotate_bmp = new wxBitmap(im_rotate_png);
	rotate_icon = new Drawable(rotating_image);
		rotate_icon->setZ(0.1);
		rotate_icon->scale(0.001);
		rotate_icon->rotate(90);
	rotating = true;
	

	

}

BasicGLPane::~BasicGLPane() {
	errorstream.close();
	delete [] win_cox, delete []  win_coy,  delete []  win_coz, delete [] obj_cox,  delete [] obj_coy,  delete [] obj_coz;
	//delete Drawable;
	delete rotating_image;
	delete rotate_icon;
	CleanupShader();
}
 
void BasicGLPane::resized(wxSizeEvent& evt)
{
    wxGLCanvas::OnSize(evt);
	
    Refresh();
}


void  BasicGLPane::processMouse(int button, int state, int x, int y) 
{
    // Used for wheels, has to be up
	if (state == GLUT_UP )
	{
		if ( button == GLUT_WHEEL_UP )
		{
			zoom += 1;
		}
		else if( button == GLUT_WHEEL_DOWN )
		{
			zoom--;
		}
	}
}


 
void BasicGLPane::prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
    /*
     *  Inits the OpenGL viewport for drawing in 3D.
     */
	
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glClearDepth(1.0f);	// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_LEQUAL); // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	
    glEnable(GL_COLOR_MATERIAL);
	
    glViewport(topleft_x, -topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
    float ratio_w_h = (float)(bottomrigth_x-topleft_x)/(float)(bottomrigth_y-topleft_y);
    gluPerspective(60/*view angle*/, ratio_w_h, 0.1 /*clip close*/, 200 /*clip far*/);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	
	
}
void BasicGLPane::camera (void) {
    glRotatef(xrot,1.0,0.0,0.0);  //rotate our camera on teh x-axis (left and right)
    glRotatef(yrot,0.0,-1.0,0.0);  //rotate our camera on the y-axis (up and down)
    glTranslated(-xpos,ypos,-zpos); //translate the screen to the position of our camera
} 

void BasicGLPane::mouseMovement(int x, int y) {
    int diffx=x-lastx; //check the difference between the current x and the last x position
    int diffy=y-lasty; //check the difference between the current y and the last y position
    lastx=x; //set lastx to the current x position
    lasty=y; //set lasty to the current y position
    xrot += (float) diffy; //set the xrot to xrot with the addition of the difference in the y position
    yrot += (float) diffx;    //set the xrot to yrot with the addition of the difference in the x position
}
void BasicGLPane::mouseshift(int x, int y) {
	float diffx=((float)x-shiftlastx) / 50.0; //check the difference between the current x and the last x position
    float diffy= -((float)y-shiftlasty) / 50.0; //check the difference between the current y and the last y position
    shiftlastx=x; //set lastx to the current x position
    shiftlasty=y; //set lasty to the current y position
	xshift += diffx;
	yshift += diffy;
}
void BasicGLPane::setFlag(int x, int y) {
	if ( x >= 0 && x < Alti.getxsize())
	Alti.FlagX = x;
	if ( y >= 0 && y < Alti.getysize())
	Alti.FlagY = Alti.getysize() - y;
	Refresh();
}


void BasicGLPane::prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
	
    /*
     *  Inits the OpenGL viewport for drawing in 2D
     */
	
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	
    glViewport(topleft_x, topleft_y, bottomrigth_x-topleft_x, bottomrigth_y-topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluOrtho2D(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
 
int BasicGLPane::getWidth()
{
    return GetSize().x;
}
 
int BasicGLPane::getHeight()
{
    return GetSize().y;
}

GLuint BasicGLPane::Maketerrain(){
	
	terrainDL = glGenLists(1); // das terrain auf den grafikkarten speicher ablegen
	
	glNewList(terrainDL,GL_COMPILE);	
	glColor4f( 0.8, 0.5 ,0.5 ,1); 
	for (double y = 0; y < Alti.getysize();++y){
		glBegin(GL_TRIANGLE_STRIP);
		for (double x = 0; x < Alti.getxsize(); ++x) {
			double var  = 1.0/(Alti.getaltitude(x,y) * Terrain_height_scale_fac  * 3.5 ); //// color fuer hoehe, je hoeher desto heller, 
			double color = ((var) > 1.0 ) ? (1.0) : var ; //wert darf nicht ueber 1
			if (y == 0){
				glColor4f( color, 0.7,0.4  ,1); 
				glTexCoord2f(0,0);
				glVertex3f(x * terrainStepWidth,y * terrainStepLength,- Alti.getaltitude(x,y) * Terrain_height_scale_fac);//
			}
			else{
				glColor4f( color,0.7 , 0.4, 1); 
				glTexCoord2f(0, (int(x)%2) ? 0 : 1);
				glVertex3f(x * terrainStepWidth ,y * terrainStepLength,- Alti.getaltitude(x,y-1)  * Terrain_height_scale_fac);// bei allen quadratic strips ausser dem ersten wird die selbe höhe wie in dem strip uzvor genommen (
				// da die vertices am gleichen platz sind. Dafür muss das letzte strip also y-1 genommen werden
			}

			 //glColor4f( 0.5, Alti.getaltitude(x,y) ,Alti.getaltitude(x,y) ,1);
			glTexCoord2f( (int(x)%2) ? 0 : 1,1);
			 glVertex3f(x * terrainStepWidth,(y+1) * terrainStepLength, -Alti.getaltitude(x,y)  * Terrain_height_scale_fac);// y+1 sehr wichtig damit vertex ueber dem ersten gebaut werden
		}
		
		glEnd();
		
	}
	glEndList();


	return(terrainDL);
}

void BasicGLPane::refresh_terrain() {
	//terrainDL_id =  Maketerrain();
	//updateTerrain();
	Update = true;

}
void BasicGLPane::defineflag() {
	//glEnable(GL_COLOR_MATERIAL);
	//glEnable(GL_LIGHT1);
	glColor4f(0.8,0.7,1,1);
	glTranslatef(Alti.FlagX * 0.025,(Alti.FlagY)  * 0.025,Alti.getaltitude(Alti.FlagX,(Alti.FlagY))  * Terrain_height_scale_fac);
	cylinder =  gluNewQuadric();
	gluCylinder(cylinder,0.05,0.05,0.5,20,20);
	//glDisable(GL_LIGHT1);
	//glDisable(GL_COLOR_MATERIAL);
}

void BasicGLPane::zoom_func(){
	GLint viewport[4];
	glGetIntegerv( GL_VIEWPORT, viewport );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix(
	viewport[2]*(.5), viewport[3]*(.5),
	viewport[2]*zoom, viewport[3]*zoom,
	viewport );

	gluPerspective( 45, double(viewport[2])/viewport[3], 0.1, 100 );
}

 
void BasicGLPane::render( wxPaintEvent& evt )
{
    if(!IsShown()) return;

    

	
    wxGLCanvas::SetCurrent();

	/*if (terrainDL_id == 0){
		terrainDL_id =  Maketerrain();}*/
		//glutCreateWindow("GLEW Test");

	if (durchlaufe == 0) { //beim ersten durchlauf
		init_textures();
		terrainCreate();
		if (!generateVertices()) 
		{
			string msg;
			msg += "Application initialization failed! could not create vertices :" ;
			Create_Ok_Dialog(msg,string("Error"));
		}
		


	}

	
    wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event
	
	
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




	
    // ------------- draw some 3D ----------------
    prepare3DViewport(0,0,getWidth(), getHeight());
	
    glLoadIdentity();
	//glutMouseFunc(processMouse);
	

	

	glUseProgram(g_terrainShader);
	UpdateTerrainShaderParameters();
	
	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
	//glLightfv(GL_LIGHT0, GL_SPECULAR, yellowAmbientDiffuse );
	glLightfv(GL_LIGHT0, GL_AMBIENT, yellowAmbientDiffuse);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, yellowAmbientDiffuse);
    //glLightfv(GL_LIGHT0, GL_POSITION, g_lightDir);

    glEnable(GL_COLOR_MATERIAL);

	
    glColor4f(0.6,0.7,1,1);
//	gluLookAt(camera[0], camera[1], camera[2], 
//		/* look from camera XYZ */ 0, 0, 0, 
//		/* look at the origin */ 0, 1, 0);

    glTranslatef(xshift,yshift,zoom);
	glRotatef(yrot,0, 1, 0); //xrotation
	glRotatef(xrot,1, 0, 0); // yrotation



	/* positive Y up vector */ 
	//glRotatef(orbitDegrees, 0.f, 1.f, 0.f);/* orbit the Y axis */ /* ...where orbitDegrees is derived from mouse motion */ 

	if (Update == true){
		updateTerrain();
		Update = false;
	}
	


	
	glEnable(GL_TEXTURE_2D);
	BindTexture(g_regions[0].texture, 0);
    BindTexture(g_regions[1].texture, 1);
    BindTexture(g_regions[2].texture, 2);
    BindTexture(g_regions[3].texture, 3);
	
	
	


	//glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   // glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(6 * sizeof(float)));
	terrainDraw();
	if (Alti.showflag == true)
	{
		glPushMatrix(); // die objekte isolieren (rtanlatef wird nur auf nachfolgende elemente angewendet
		defineflag(); //rendert positionsanzeiger
		glPopMatrix(); //operationen wieder auf die ganze szene anwenden
	}
	
   
    for (int i = 3; i >= 0; --i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }
	
	//if (rotating){
	
	//}
	//camera();


	glUseProgram(0);
	
	//glEnable(GL_TEXTURE_2D);
	//	rotate_icon->render();
	glDisable(GL_LIGHT0);
    glDisable(GL_LIGHTING);
	

	
	

	


    glFlush();
    SwapBuffers();
	durchlaufe += 1;

	

}

void BasicGLPane::Create_Ok_Dialog(string msg,string title){
	
	Ok_dialog = new wxMessageDialog(NULL,wxT(msg),wxT(title),wxOK|wxICON_HAND,wxDefaultPosition);
	Ok_dialog->ShowModal();
	Ok_dialog->Destroy();
}
void BasicGLPane::init_textures(void) {
	//kann erst aktiviert werden,
	//nachdem das programm gestartet (dh implement app aufgerufen) wurde.
	
		GLenum err = glewInit();

		if (GLEW_OK != err){
			string msg;

			msg += "Application initialization failed! wrong opengl version ? :" ;
			//msg += reinterpret_cast<char*>(glewGetErrorString(err));
			msg += (char*) glGetString(GL_VERSION);
				/* Problem: glewInit failed, something is seriously wrong. */
			//fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
			Create_Ok_Dialog(msg,string("Error"));
			//if (true) {throw std::runtime_error("Quitting");}
		}

		try {

			initstruct();
			create_textures();
			}

		catch (const std::exception &e){
			string msg;

			msg += "Application initialization failed! /n" ;
			msg += e.what();
			Create_Ok_Dialog(msg,string("Error"));
		}
}

void BasicGLPane::updateTerrain(void) {
		terrainCreate();
		if (!generateVertices()) 
		{
			string msg;
			msg += "Application initialization failed! could not create vertices :" ;
			Create_Ok_Dialog(msg,string("Error"));
		}

}

void BasicGLPane::Start_WaterSimulation(int method){

	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer );
	//den buffer kurz freigeben und dann der water simulation das vertex array uebergeben, anschliessend
	// den buffer wieder "in die haende" von OpenGl geben
	Water_Simulation watersim = Water_Simulation(&Water_Kcs, method);

	glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


}
