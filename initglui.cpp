
//
// initialize the glui window:
//

GLUI* GluiMain;

const int MSEC = 1;
float BackgroundIntensity = 0.5f; 
char BGFORMAT[] = "Background Intensity: %.2f"; // Format for background intensity display

float ProjRotMatrix[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 }; // Projection rotation matrix
float ProjScale2 = 1.0f;            // Projection scale factor
float ProjTransXYZ[3] = { 0.0f, 0.0f, 0.0f }; // Projection translation

float EyeRotMatrix[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };  // Eye rotation matrix
float EyeScale2 = 1.0f;             // Eye scale factor
float EyeTransXYZ[3] = { 0.0f, 0.0f, 0.0f }; // Eye translation

void SetBackgroundIntensity(int id) {}

void
GluiIdle(void)
{
	if ( glutGetWindow() != MainWindow ) 
    	glutSetWindow(MainWindow);  
	
	glutPostRedisplay();
}

void
Refresh(int val)
{
	glutPostRedisplay();
	glutTimerFunc( MSEC, Refresh, val );
}

void Buttons(int id)
{
	switch (id)
	{
	case RESET:
		Reset();
		break;

	case QUIT:
		// gracefully close out the graphics:
		// gracefully close the graphics window:
		// gracefully exit the program:
		glutSetWindow(MainWindow);
		glFinish();
		glutDestroyWindow(MainWindow);
		exit(0);
		break;

	case ADD:
		addMoreParticles(500);
		break;

	default:
		fprintf(stderr, "Don't know what to do with Button ID %d\n", id);
	}
	glutSetWindow(MainWindow);
}

void
InitGluiMain( void )
{
	GLUI_Panel *panel, *panel2;
	GLUI_Rollout *rollout;
	GLUI_Rotation *rot;
	GLUI_Translation *trans, *scale;

	// setup the glui window:

	glutInitWindowPosition( glutGet(GLUT_WINDOW_WIDTH) + 50, 0 );
	GluiMain = GLUI_Master.create_glui( (char *) GLUITITLE );

	GluiMain->add_statictext( (char *) GLUITITLE );
	GluiMain->add_separator();

	panel = GluiMain->add_panel( "", true );
	GluiMain->add_checkbox_to_panel( panel, "Axes", &AxesOn );
	GluiMain->add_column_to_panel( panel, GLUIFALSE );
	GluiMain->add_checkbox_to_panel( panel, "Perspective", &NowProjection );

	GLUI_Spinner* spinner = GluiMain->add_spinner_to_panel(
		panel,
		"Background Intensity",
		GLUI_SPINNER_FLOAT,
		&BackgroundIntensity,
		1,
		(GLUI_Update_CB)SetBackgroundIntensity
	);
	// Set spinner limits
	spinner->set_float_limits(0.0f, 1.0f, GLUI_LIMIT_CLAMP);

	// panel = GluiMain->add_panel( "", true );
	// GLUI_HSlider *slider = GluiMain->add_slider_to_panel( panel, false, GLUI_HSLIDER_FLOAT, &BackgroundIntensity, 0, (GLUI_Update_CB) SetBackgroundIntensity );
	// slider->set_float_limits( 0., 1. );
	// slider->set_w( 200 );
	// char str[128];
	// sprintf( str, BGFORMAT, BackgroundIntensity );
	// BgLabel = GluiMain->add_statictext_to_panel( panel, str );

	// // GluiMain->add_checkbox( "Intensity Depth Cue", &DepthCueOn );

	GluiMain->add_checkbox("Display Frame Rate", &DisplayFrameRate);

	panel = GluiMain->add_panel("Global (Projection Matrix) Transformation");


	panel2 = panel;
	rot = GluiMain->add_rotation_to_panel(panel2, "Rotation", (float*)ProjRotMatrix, 0);
	rot->set_spin(1.0);

	GluiMain->add_column_to_panel(panel2, GLUIFALSE);
	scale = GluiMain->add_translation_to_panel(panel2, "Scale", GLUI_TRANSLATION_Y, &ProjScale2);
	scale->set_speed(0.01f);

	GluiMain->add_column_to_panel(panel2, GLUIFALSE);
	trans = GluiMain->add_translation_to_panel(panel2, "Trans XY", GLUI_TRANSLATION_XY, &ProjTransXYZ[0]);
	trans->set_speed(0.01f);

	GluiMain->add_column_to_panel(panel2, GLUIFALSE);
	trans = GluiMain->add_translation_to_panel(panel2, "Trans Z", GLUI_TRANSLATION_Z, &ProjTransXYZ[2]);
	trans->set_speed(0.01f);


	panel = GluiMain->add_panel("Eye (ModelView Matrix) Transformation");

	panel2 = panel;
	rot = GluiMain->add_rotation_to_panel(panel2, "Rotation", (float*)EyeRotMatrix, 0);
	rot->set_spin(1.0);

	GluiMain->add_column_to_panel(panel2, GLUIFALSE);
	scale = GluiMain->add_translation_to_panel(panel2, "Scale", GLUI_TRANSLATION_Y, &EyeScale2);
	scale->set_speed(0.01f);

	GluiMain->add_column_to_panel(panel2, GLUIFALSE);
	trans = GluiMain->add_translation_to_panel(panel2, "Trans XY", GLUI_TRANSLATION_XY, &EyeTransXYZ[0]);
	trans->set_speed(0.01f);

	GluiMain->add_column_to_panel(panel2, GLUIFALSE);
	trans = GluiMain->add_translation_to_panel(panel2, "Trans Z", GLUI_TRANSLATION_Z, &EyeTransXYZ[2]);
	trans->set_speed(0.01f);

	GluiMain->add_checkbox("Verbose", &Verbose);


	panel = GluiMain->add_panel("", GLUIFALSE);

	GluiMain->add_button_to_panel(panel, "Reset", RESET, (GLUI_Update_CB)Buttons);

	GluiMain->add_column_to_panel(panel, GLUIFALSE);

	GluiMain->add_button_to_panel(panel, "Quit", QUIT, (GLUI_Update_CB)Buttons);

	// tell glui what graphics window it needs to post a redisplay to:

	if (MainWindow >= 0)
		GluiMain->set_main_gfx_window(MainWindow);


	// set the graphics window's idle function:

	// GLUI_Master.set_glutIdleFunc( NULL );
	GLUI_Master.set_glutIdleFunc(GluiIdle);

	// glutSetWindow( GluiMain->get_glut_window_id() );
	glutTimerFunc(MSEC, Refresh, MSEC);


	// give the glui windows the same kb callback as the graphics windows
	// this makes the kb command shortcuts more useable:

	// GLUI_Master.set_glutKeyboardFunc( Keyboard );
}