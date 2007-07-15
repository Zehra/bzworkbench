#include "../include/windows/MainWindow.h"

bool MainWindow::initialized = false;

// default constructor
MainWindow::MainWindow() :
	Fl_Window(DEFAULT_WIDTH, DEFAULT_HEIGHT, "BZWorkbench") {
	
	this->end();
	
	this->model = new Model();
	printf("2\n");
	this->view = new View(model, this, RENDER_WINDOW_X, RENDER_WINDOW_Y, RENDER_WINDOW_WIDTH, RENDER_WINDOW_HEIGHT);
	printf("3\n");
	view->end();
	
	this->add(view);
	
	this->mainMenu = new MainMenu(this, MENU_X, MENU_Y, MENU_WIDTH, MENU_HEIGHT);
	this->add(mainMenu);
	
	initialized = true;
   
   // add the View as an observer to the Model
   model->addObserver( view );
   
}

// construct from a model
MainWindow::MainWindow(Model* model) :
	Fl_Window(DEFAULT_WIDTH, DEFAULT_HEIGHT, "BZWorkbench") {
	
	
	this->end();
	
	this->model = model;
	
	this->view = new View(model, this, RENDER_WINDOW_X, RENDER_WINDOW_Y, RENDER_WINDOW_WIDTH, RENDER_WINDOW_HEIGHT);
	view->end();
	this->add(view);
	
	this->mainMenu = new MainMenu(this, MENU_X, MENU_Y, MENU_WIDTH, MENU_HEIGHT);
	this->add(mainMenu);
	
	
	initialized = true;
   
   // add the View as an observer to the Model
   model->addObserver( view );
	
}

MainWindow::~MainWindow() {
	if(model)
		delete model;
	
	initialized = false;
	
}

// handle
int MainWindow::handle(int event) {
	
	switch( event ) {
		// catch right-click pushes--the user should double-click to access the menu
		case FL_PUSH:
			if( Fl::event_button() == FL_RIGHT_MOUSE) {
				if( Fl::event_clicks() > 0 ) {
					return Fl_Window::handle( event );
				}
				else {
					return Fl::handle( event, this->view );
				}
			}
			return Fl_Window::handle( event );
			
		case FL_CLOSE:	// catch window close event
			// make sure to de-allocate nodes
			SceneBuilder::shutdown();
			return 1;
		default:
				
			return Fl_Window::handle( event );
	}
}

// launch a MasterConfigurationDialog
void MainWindow::configure( bz2object* obj ) {
	MasterConfigurationDialog* mcd = new MasterConfigurationDialog( obj );
	mcd->show();
}
