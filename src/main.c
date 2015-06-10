#include <pebble.h>

#define DRUGS_NUM_SECTIONS 1
#define DRUGS_NUM_ITEMS 5
#define LABS_NUM_SECTIONS 4
#define LABS_BLOOD_ROWS 8
#define LABS_LYTE_ROWS 10
#define LABS_MOL_ROWS 8
#define LABS_COAG_ROWS 3
  
static Window *s_main_window;
static Window *s_drugs_window;
static Window *s_ind_drug_window;
static Window *s_labs_window;
static TextLayer *s_main_window_params_layer;
static TextLayer *s_main_window_time_layer;
static TextLayer *s_main_window_seconds_layer;
static TextLayer *s_main_window_day_layer;
static TextLayer *s_main_window_date_layer;
static TextLayer *s_main_window_month_layer;
static TextLayer *s_main_window_labs_layer;
static TextLayer *s_ind_drug_info_text_layer;
static ScrollLayer *s_ind_drug_info_scroll_layer;
static SimpleMenuLayer *s_drugs_menu_layer;
static SimpleMenuLayer *s_labs_menu_layer;
static SimpleMenuSection s_drugs_menu_sections[DRUGS_NUM_SECTIONS];
static SimpleMenuSection s_labs_menu_sections[LABS_NUM_SECTIONS];
static SimpleMenuItem s_drugs_menu_items[DRUGS_NUM_ITEMS];
static SimpleMenuItem s_labs_menu_section_bloods[LABS_BLOOD_ROWS];
static SimpleMenuItem s_labs_menu_section_lytes[LABS_LYTE_ROWS];
static SimpleMenuItem s_labs_menu_section_mols[LABS_MOL_ROWS];
static SimpleMenuItem s_labs_menu_section_coags[LABS_COAG_ROWS];
static char s_drug_information[1000];

//Ideas?
//Code Mode.

//Display resolution: 144x168.


//Individual drug window handlers.
static void ind_drug_window_load(Window *window) {
  APP_LOG(APP_LOG_LEVEL_INFO,"Starting ind_drug_window_load(). Creating scroll layer and setting click configs.");
  
  s_ind_drug_info_scroll_layer = scroll_layer_create(layer_get_frame(window_get_root_layer(window)));
  //Sets the up and down button actions to scrolling actions.
  scroll_layer_set_click_config_onto_window(s_ind_drug_info_scroll_layer,window);
  //Trims the scroll box to fit the contents of the text layer. See the scroll layer demo.
  APP_LOG(APP_LOG_LEVEL_INFO,"Getting sizes and assigning them to variables. Then setting content size of the scroll layer.");
  //This is where everything is crashing.
  int16_t scroll_layer_width = layer_get_frame(window_get_root_layer(window)).size.w;
  int16_t scroll_layer_height = text_layer_get_content_size(s_ind_drug_info_text_layer).h + 4;
  scroll_layer_set_content_size(s_ind_drug_info_scroll_layer,GSize(scroll_layer_width,scroll_layer_height));
  
  APP_LOG(APP_LOG_LEVEL_INFO,"Creating text layer and setting its values.");
  s_ind_drug_info_text_layer = text_layer_create(layer_get_frame(window_get_root_layer(window)));
  text_layer_set_font(s_ind_drug_info_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text(s_ind_drug_info_text_layer,s_drug_information);
  //Child assignment.
  APP_LOG(APP_LOG_LEVEL_INFO,"Assigning children.");
  scroll_layer_add_child(s_ind_drug_info_scroll_layer,text_layer_get_layer(s_ind_drug_info_text_layer));
  layer_add_child(window_get_root_layer(window),scroll_layer_get_layer(s_ind_drug_info_scroll_layer));
}

static void ind_drug_window_unload(Window *window) {
  text_layer_destroy(s_ind_drug_info_text_layer);
  window_destroy(s_ind_drug_window);
}

//Function to create the window that will display the drug information. This seemed easier than having
//one callback function for each drug name, as well as multiple windows and load functions.
//!!Must register this as a callback with all drug menu items.
static void ind_drug_window_create(int index,void *ctx) {
  s_ind_drug_window = window_create();
  switch(index) {
    case 0:
      strcpy(s_drug_information,"Amiodarone\n\nClass: Class III Antiarrhythmic\n\nPharmacodynamics: Prolongs cardiac repolarization.\n\nIV Loading Dose: 150-450mg");
    break;
    case 1:
      strcpy(s_drug_information,"Atropine\nClass: Muscarinic Anticholinergic\nPharmacodynamics: Inhibits parasympathetic activity. Treats bradycardia. No longer recommended for asystole/PEA.\nIV Dose: 0.5-1mg push");
    break;
  }
  window_set_window_handlers(s_ind_drug_window,(WindowHandlers) {.load=ind_drug_window_load,.unload=ind_drug_window_unload});
  window_stack_push(s_ind_drug_window,true);
}


//Drugs window.
static void drugs_window_load(Window *window) {
  //Debug statement.
  //APP_LOG(APP_LOG_LEVEL_INFO,"Drugs window load function activated.");
  //Setup counting integer.
  int drug_menu_count = 0;
  //Drug names.
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Amiodarone",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Atropine",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Diltiazem"
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Dobutamine"
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Dopamine"
  };
  
  s_drugs_menu_sections[0] = (SimpleMenuSection) {
    .title = NULL,
    .num_items = DRUGS_NUM_ITEMS,
    .items = s_drugs_menu_items
  };
  
  //Debug statement.
  //APP_LOG(APP_LOG_LEVEL_INFO,"About to add the drug layer as a child. Current memory usage is %u. %u free.",heap_bytes_used(),heap_bytes_free());
  s_drugs_menu_layer = simple_menu_layer_create(layer_get_frame(window_get_root_layer(window)),window,s_drugs_menu_sections,DRUGS_NUM_SECTIONS,NULL);
  layer_add_child(window_get_root_layer(window),simple_menu_layer_get_layer(s_drugs_menu_layer));
  //APP_LOG(APP_LOG_LEVEL_INFO,"Added layer as child.");
}

static void drugs_window_unload(Window *window) {
  simple_menu_layer_destroy(s_drugs_menu_layer);
  window_destroy(window);
}

//Labs window.
static void labs_window_load(Window *window) {
  //Create counting integer to load the blood menu items.
  int blood_menu_count = 0;
  //Blood values.  
  s_labs_menu_section_bloods[blood_menu_count++] = (SimpleMenuItem) {
    .title = "Hemoglobin",
    .subtitle = "140-160 mg/dL"
  };
  s_labs_menu_section_bloods[blood_menu_count++] = (SimpleMenuItem) {
    .title = "Hematocrit",
    .subtitle = "0.40-0.50"
  };
  s_labs_menu_section_bloods[blood_menu_count++] = (SimpleMenuItem) {
    .title = "White Count",
    .subtitle = "3.5-11 x 10^9/L"
  };
  s_labs_menu_section_bloods[blood_menu_count++] = (SimpleMenuItem) {
    .title = "Platelets",
    .subtitle = "150-400 x 10^9/L"
  };
  s_labs_menu_section_bloods[blood_menu_count++] = (SimpleMenuItem) {
    .title = "Neutrophils",
    .subtitle = "2-8 x 10^9/L"
  };
  s_labs_menu_section_bloods[blood_menu_count++] = (SimpleMenuItem) {
    .title = "Lymphocytes",
    .subtitle = "1-4 x 10^9/L"
  };
  s_labs_menu_section_bloods[blood_menu_count++] = (SimpleMenuItem) {
    .title = "Monocytes",
    .subtitle = "0.2-0.8 x 10^9/L"
  };
  s_labs_menu_section_bloods[blood_menu_count++] = (SimpleMenuItem) {
    .title= "Eosinophils",
    .subtitle = "<0.5 x 10^9/L"
  };
  
  //Lytes
  int lyte_menu_count = 0;
  s_labs_menu_section_lytes[lyte_menu_count++] = (SimpleMenuItem) {
    .title = "Sodium",
    .subtitle = "135-145 mmol/L"
  };
  s_labs_menu_section_lytes[lyte_menu_count++] = (SimpleMenuItem) {
    .title = "Potassium",
    .subtitle = "3.5-5.0 mmol/L"
  };
  s_labs_menu_section_lytes[lyte_menu_count++] = (SimpleMenuItem) {
    .title = "Chloride",
    .subtitle = "95-105 mmol/L"
  };
  s_labs_menu_section_lytes[lyte_menu_count++] = (SimpleMenuItem) {
    .title = "Bicarbonate",
    .subtitle = "18-22 mmol/L"
  };
  s_labs_menu_section_lytes[lyte_menu_count++] = (SimpleMenuItem) {
    .title = "Creatinine",
    .subtitle = "(F) 45-90 (M) 60-110 umol/L"
  };
  s_labs_menu_section_lytes[lyte_menu_count++] = (SimpleMenuItem) {
    .title = "Glucose",
    .subtitle = "(ND) 4-7 (D) 5-10 mmol/L"
  };
  s_labs_menu_section_lytes[lyte_menu_count++] = (SimpleMenuItem) {
    .title = "Blood Urea Nitrogen",
    .subtitle = "3-7 mmol/L"
  };
  s_labs_menu_section_lytes[lyte_menu_count++] = (SimpleMenuItem) {
    .title = "Calcium",
    .subtitle = "2-2.6 mmol/L?"
  };
  s_labs_menu_section_lytes[lyte_menu_count++] = (SimpleMenuItem) {
    .title = "Magnesium",
    .subtitle = "1.5-2 mEq/L"
  };
  s_labs_menu_section_lytes[lyte_menu_count++] = (SimpleMenuItem) {
    .title = "Phosphate",
    .subtitle = "0.8-1.5 mmol/L"
  };
  
  //Blood gases.
  int mol_menu_count = 0;
  s_labs_menu_section_mols[mol_menu_count++] = (SimpleMenuItem) {
    .title = "ALP",
    .subtitle = "50-100 IU/L"
  };
  s_labs_menu_section_mols[mol_menu_count++] = (SimpleMenuItem) {
    .title = "ALT",
    .subtitle = "5-30 IU/L"
  };
  s_labs_menu_section_mols[mol_menu_count++] = (SimpleMenuItem) {
    .title = "AST",
    .subtitle = "5-30 IU/L"
  };
  s_labs_menu_section_mols[mol_menu_count++] = (SimpleMenuItem) {
    .title = "Lactate",
    .subtitle = "<2.5 mmol/L"
  };
  s_labs_menu_section_mols[mol_menu_count++] = (SimpleMenuItem) {
    .title = "Troponin",
    .subtitle = "<15 ng/L"
  };
  s_labs_menu_section_mols[mol_menu_count++] = (SimpleMenuItem) {
    .title = "CRP",
    .subtitle = "<5 mg/L"
  };
  s_labs_menu_section_mols[mol_menu_count++] = (SimpleMenuItem) {
    .title = "PaO2",
    .subtitle = "80-100 mmHg"
  };
  s_labs_menu_section_mols[mol_menu_count++] = (SimpleMenuItem) {
    .title = "PaCO2",
    .subtitle = "35-45 mmHg"
  };
  
  //Coagulation values.
  int coag_menu_count = 0;
  s_labs_menu_section_coags[coag_menu_count++] = (SimpleMenuItem) {
    .title = "Prothrombin Time",
    .subtitle = "11-14 s"
  };
  s_labs_menu_section_coags[coag_menu_count++] = (SimpleMenuItem) {
    .title = "Partial Thromboplastin Time",
    .subtitle = "25-35 s"
  };
  s_labs_menu_section_coags[coag_menu_count++] = (SimpleMenuItem) {
    .title = "International Normalized Ratio",
    .subtitle = "1.0"
  };
    
  //Section headers.
  s_labs_menu_sections[0] = (SimpleMenuSection) {
    .title = "Blood Values",
    .num_items = LABS_BLOOD_ROWS,
    .items = s_labs_menu_section_bloods
  };
  s_labs_menu_sections[1] = (SimpleMenuSection) {
    .title = "SMA10",
    .num_items = LABS_LYTE_ROWS,
    .items = s_labs_menu_section_lytes
  };
  s_labs_menu_sections[2] = (SimpleMenuSection) {
    .title = "Molecules & Gases",
    .num_items = LABS_MOL_ROWS,
    .items = s_labs_menu_section_mols
  };
  
  s_labs_menu_layer = simple_menu_layer_create(layer_get_frame(window_get_root_layer(window)),window,s_labs_menu_sections,LABS_NUM_SECTIONS,NULL);
  
  layer_add_child(window_get_root_layer(window),simple_menu_layer_get_layer(s_labs_menu_layer));
}

static void labs_window_unload(Window *window) {
  simple_menu_layer_destroy(s_labs_menu_layer);
  window_destroy(window);
}

//Click handlers.

static void main_window_down_click_handler(ClickRecognizerRef recognizer,void *context) {
  //Common lab values.
  s_labs_window = window_create();
  window_set_window_handlers(s_labs_window,(WindowHandlers) {.load=labs_window_load,.unload=labs_window_unload});
  window_stack_push(s_labs_window,true);
}

static void main_window_up_click_handler(ClickRecognizerRef recognizer,void *context) {
  //Debug statement.
  //APP_LOG(APP_LOG_LEVEL_INFO,"Up handler activated.");
  //Common medications.
  s_drugs_window = window_create();
  window_set_window_handlers(s_drugs_window,(WindowHandlers) {.load=drugs_window_load,.unload=drugs_window_unload});
  window_stack_push(s_drugs_window,true);
}

static void main_window_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_DOWN,main_window_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP,main_window_up_click_handler);
}

//Main window handler.

static void main_window_load(Window *window) {
  window_set_background_color(window,GColorBlack);
  window_set_click_config_provider(s_main_window,main_window_click_config_provider);
  //Create TextLayers.
  s_main_window_params_layer = text_layer_create(GRect(0,0,144,20));
  s_main_window_time_layer = text_layer_create(GRect(0,45,114,50));
  s_main_window_seconds_layer = text_layer_create(GRect(120,60,24,35));
  s_main_window_day_layer = text_layer_create(GRect(0,101,30,20));
  s_main_window_date_layer = text_layer_create(GRect(36,101,22,20));
  s_main_window_month_layer = text_layer_create(GRect(64,101,80,20));
  s_main_window_labs_layer = text_layer_create(GRect(0,148,144,20));
  //Params formatting.
  text_layer_set_text(s_main_window_params_layer,"Rx");
  text_layer_set_text_alignment(s_main_window_params_layer,GTextAlignmentRight);
  text_layer_set_background_color(s_main_window_params_layer,GColorBlack);
  text_layer_set_text_color(s_main_window_params_layer,GColorWhite);
  //Time formatting.
  text_layer_set_text_alignment(s_main_window_time_layer,GTextAlignmentCenter);
  text_layer_set_font(s_main_window_time_layer,fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
  //Seconds formatting.
  text_layer_set_text_alignment(s_main_window_seconds_layer,GTextAlignmentCenter);
  text_layer_set_font(s_main_window_seconds_layer,fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_main_window_seconds_layer,"00");
  //Day formatting.
  text_layer_set_text_alignment(s_main_window_day_layer,GTextAlignmentCenter);
  text_layer_set_font(s_main_window_day_layer,fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  //Date formatting.
  text_layer_set_text_alignment(s_main_window_date_layer,GTextAlignmentCenter);
  text_layer_set_font(s_main_window_date_layer,fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  //Month formatting.
  text_layer_set_text_alignment(s_main_window_month_layer,GTextAlignmentRight);
  //Labs formatting.
  text_layer_set_text(s_main_window_labs_layer,"Labs");
  text_layer_set_text_alignment(s_main_window_labs_layer,GTextAlignmentRight);
  text_layer_set_background_color(s_main_window_labs_layer,GColorBlack);
  text_layer_set_text_color(s_main_window_labs_layer,GColorWhite);
  //Attach TextLayers to main window.
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_main_window_params_layer));
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_main_window_time_layer));
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_main_window_seconds_layer));
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_main_window_day_layer));
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_main_window_date_layer));
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_main_window_month_layer));
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_main_window_labs_layer));
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_main_window_params_layer);
  text_layer_destroy(s_main_window_time_layer);
  text_layer_destroy(s_main_window_seconds_layer);
  text_layer_destroy(s_main_window_day_layer);
  text_layer_destroy(s_main_window_date_layer);
  text_layer_destroy(s_main_window_month_layer);
  text_layer_destroy(s_main_window_labs_layer);
}

//Time handler. For better resource management you could make two: one for seconds and one for minutes/hours.

static void update_time() {
  //Create a time container and a pointer to it. Then assign it the current time.
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char time_buffer[] = "00:00";
  static char second_buffer[] = "00";
  static char month_buffer[] = "September";
  static char date_buffer[] = "00";
  static char day_buffer[] = "Sun";
  if(clock_is_24h_style() == true) {
    strftime(time_buffer,sizeof("00:00"),"%H:%M",tick_time);
  } else {
    strftime(time_buffer,sizeof("00:00"),"%I:%M",tick_time);
  }
  strftime(second_buffer,sizeof("00"),"%S",tick_time);
  strftime(month_buffer,sizeof("September"),"%B",tick_time);
  strftime(date_buffer,sizeof("00"),"%e",tick_time);
  strftime(day_buffer,sizeof("Sun"),"%a",tick_time);
  
  text_layer_set_text(s_main_window_time_layer,time_buffer);
  text_layer_set_text(s_main_window_seconds_layer,second_buffer);
  text_layer_set_text(s_main_window_date_layer,date_buffer);
  text_layer_set_text(s_main_window_month_layer,month_buffer);
  text_layer_set_text(s_main_window_day_layer,day_buffer);
}

static void tick_handler(struct tm *tick_time,TimeUnits units_changed) {
  update_time();
}

static void init() {
  tick_timer_service_subscribe(SECOND_UNIT,tick_handler);
  s_main_window = window_create();
  window_set_window_handlers(s_main_window,(WindowHandlers) {.load=main_window_load,.unload=main_window_unload});
  window_set_fullscreen(s_main_window,true);
  //Display main window on screen on launch.
  window_stack_push(s_main_window,true);
  update_time();
}

static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}