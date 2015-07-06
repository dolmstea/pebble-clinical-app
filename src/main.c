#include <pebble.h>

#define DRUGS_NUM_SECTIONS 1
#define DRUGS_NUM_ITEMS 17
#define LABS_NUM_SECTIONS 4
#define LABS_BLOOD_ROWS 8
#define LABS_LYTE_ROWS 10
#define LABS_MOL_ROWS 8
#define LABS_COAG_ROWS 3
  
static Window *s_main_window;
static Window *s_drugs_window;
static Window *s_ind_drug_window;
static Window *s_labs_window;
static Window *s_code_window;
static TextLayer *s_main_window_params_layer;
static TextLayer *s_main_window_time_layer;
static TextLayer *s_main_window_seconds_layer;
static TextLayer *s_main_window_day_layer;
static TextLayer *s_main_window_date_layer;
static TextLayer *s_main_window_month_layer;
static TextLayer *s_main_window_battery_layer;
static TextLayer *s_main_window_battery_bar_layer;
static TextLayer *s_main_window_labs_layer;
static TextLayer *s_ind_drug_info_text_layer;
static TextLayer *s_resus_title_text_layer;
static TextLayer *s_resus_clock_text_layer;
static TextLayer *s_resus_timer_text_layer;
static TextLayer *s_resus_shock_icon_text_layer;
static TextLayer *s_resus_cpr_icon_text_layer;
static TextLayer *s_resus_drug_icon_text_layer;
static TextLayer *s_resus_log_text_layer;
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
static char s_resus_log[1000];
static int s_resus_timer_seconds = 0;
static int s_battery_bar_width = 0;
static int s_battery_bar_max_width = 110;

//Early function declarations.
static void main_window_select_long_click_handler(ClickRecognizerRef recognizer,void *context);

//Ideas?
//Code Mode.
//Vibrates 100bpm. Vibration could be toggled by the select button?

//Battery bar does not work yet.

//Display resolution: 144x168.

//Time handlers. For better resource management you could make two: one for seconds and one for minutes/hours.

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

static void resus_update_time() {
  //This function is basically a repeat of update_time but will be subscribed in code mode so that the timer
  //can start as soon as the window is loaded rather than when the app is loaded.
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  //Format the time buffer for 24h or 12h time.
  static char time_buffer[] = "00:00:00";
  if(clock_is_24h_style() == true) {
    strftime(time_buffer,sizeof("00:00:00"),"%T",tick_time);
  } else {
    strftime(time_buffer,sizeof("00:00:00"),"%I:%M:%S",tick_time);
  }
  
  //Set the resus clock text.
  text_layer_set_text(s_resus_clock_text_layer,time_buffer);
  
  //Update and increment the resus timer seconds.
  static char timer_buffer[] = "00:00";
  snprintf(timer_buffer,sizeof(timer_buffer),"%d:%02d",s_resus_timer_seconds / 60,s_resus_timer_seconds % 60);
  text_layer_set_text(s_resus_timer_text_layer,timer_buffer);
  s_resus_timer_seconds++;
}

static void tick_handler(struct tm *tick_time,TimeUnits units_changed) {
  update_time();
}

static void resus_tick_handler(struct tm *tick_time,TimeUnits units_changed) {
  resus_update_time();
}

//Resus mode click handlers.
static void resus_window_up_click_handler(ClickRecognizerRef recognizer,void *context) {
  //Initialize a time struct and format it into a second-denominated string.
  //Could possibly set this up to record in 12h time if the watch is set that way.
  time_t temp = time(NULL);
  struct tm *time = localtime(&temp);
  static char timestamp_buffer[] = "00:00:00";
  strftime(timestamp_buffer,sizeof("00:00:00"),"%T",time);
  
  //Format the addition to the log.
  static char log_entry_buffer[] = "00:00:00  Shock\n";
  snprintf(log_entry_buffer,sizeof(log_entry_buffer),"%s  Shock\n",timestamp_buffer);
  
  //Append the log to include the new string.
  strcat(s_resus_log,log_entry_buffer);
  
  //Update the resus log.
  text_layer_set_text(s_resus_log_text_layer,s_resus_log);
}

static void resus_window_down_click_handler(ClickRecognizerRef recognizer,void *context) {
  //Initialize a time struct and format it into a second-denominated string.
  //Could possibly set this up to record in 12h time if the watch is set that way.
  time_t temp = time(NULL);
  struct tm *time = localtime(&temp);
  static char timestamp_buffer[] = "00:00:00";
  strftime(timestamp_buffer,sizeof("00:00:00"),"%T",time);
  
  //Format the addition to the log.
  static char log_entry_buffer[] = "00:00:00  Rx Admin\n";
  snprintf(log_entry_buffer,sizeof(log_entry_buffer),"%s  Rx Admin\n",timestamp_buffer);
  
  //Append the log to include the new string.
  strcat(s_resus_log,log_entry_buffer);
  
  //Update the resus log.
  text_layer_set_text(s_resus_log_text_layer,s_resus_log);
}

static void resus_window_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP,resus_window_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN,resus_window_down_click_handler);
}

//Individual drug window click config provider.
static void ind_drug_window_click_config_provider(void *context) {
  window_long_click_subscribe(BUTTON_ID_SELECT,0,main_window_select_long_click_handler,NULL);
}

//Individual drug window handlers.
static void ind_drug_window_load(Window *window) {
  //Initialize the TextLayer that will contain the drug information.
  GRect bounds = layer_get_frame(window_get_root_layer(window));
  s_ind_drug_info_text_layer = text_layer_create(GRect(0,0,bounds.size.w,2000));
  text_layer_set_font(s_ind_drug_info_text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text(s_ind_drug_info_text_layer,s_drug_information);
  
  //Create the scroll layer.
  s_ind_drug_info_scroll_layer = scroll_layer_create(layer_get_frame(window_get_root_layer(window)));
  //Sets the up and down button actions to scrolling actions.
  scroll_layer_set_click_config_onto_window(s_ind_drug_info_scroll_layer,window);
  //Sets the select button to activate code mode.
  scroll_layer_set_callbacks(s_ind_drug_info_scroll_layer,(ScrollLayerCallbacks) {.click_config_provider=ind_drug_window_click_config_provider});
  
  //Trims the scroll box to fit the contents of the text layer. See the scroll layer demo.
  int16_t scroll_layer_width = (layer_get_frame(window_get_root_layer(window))).size.w;
  int16_t scroll_layer_height = (text_layer_get_content_size(s_ind_drug_info_text_layer)).h + 4;
  scroll_layer_set_content_size(s_ind_drug_info_scroll_layer,GSize(scroll_layer_width,scroll_layer_height));
  
  //Child assignment.
  scroll_layer_add_child(s_ind_drug_info_scroll_layer,text_layer_get_layer(s_ind_drug_info_text_layer));
  layer_add_child(window_get_root_layer(window),scroll_layer_get_layer(s_ind_drug_info_scroll_layer));
}

static void ind_drug_window_unload(Window *window) {
  strcpy(s_drug_information,"");
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
      snprintf(s_drug_information,sizeof(s_drug_information),"%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s",
        "Amiodarone",
        "Class: Class III Antiarrhythmic",
        "Pharmacodynamics: Prolongs cardiac repolarization. Activity in all four Vaughan-Williams categories.",
        "Vent. Arrhythmias",
        "IV Loading Dose: 150mg/10min then 360mg/6hr",
        "Maintenance IV Rate: 0.5mg/min",
        "ACLS IV/IO Dose: 300mg STAT then 150mg/10min",
        "A. Fib",
        "IV Dose: 125mg/hr for 24hrs");
    break;
    case 1:
      snprintf(s_drug_information,sizeof(s_drug_information),"%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s",
        "Atropine",
        "Class: Muscarinic Anticholinergic",
        "Pharmacodynamics: Inhibits parasympathetic activity. Positive chronotropic, treats bradycardia. Also a co-antidote with pralidoxime for organophosphate (VX) poisoning.",
        "ACLS",
        "Asystole/PEA IV/IO Dose: 1mg push q 3-5 min (max 3mg)",
        "Bradycardia IV Dose: 0.5mg push q 3-5 min (max 3mg)",
        "Concomitant use with pralidoxime for organophosphate poisoning.");
    break;
    case 2:
      snprintf(s_drug_information,sizeof(s_drug_information),"%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s",
        "Diltiazem (Cardizem)",
        "Class: Calcium Channel Blocker. Class IV Antiarrhythmic.",
        "Pharmacodynamics: Relaxes arterial smooth muscles. Can also prolong cardiac conduction by inhibiting AV node depolarization. Can convert atrial arrhythmias. Negative inotrope, chronotrope & dromotrope. Antihypertensive agent.",
        "SV Tachyarrhythmia, A.Fib & A.Flutter",
        "IV Dose: 0.25mg/kg over 2 min, wait 15 mins",
        "Second IV Dose: 0.35mg/kg over 2 min",
        "Maintenance IV Rate: 5-15 mg/hr titrated to HR");
    break;
    case 3:
      snprintf(s_drug_information,sizeof(s_drug_information),"%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s",
        "Dobutamine",
        "Class: Inotrope",
        "Pharmacodynamics: Beta-1 agonist. Increases cardiac contractility. Can also reduce SVR thru vasodilation.",
        "Cardiac Decompensation, Shock & ACLS",
        "Initiation IV Rate: 0.5-1 mcg/kg/min",
        "Maintenance IV Rate: 2-20 mcg/kg/min");
    break;
    case 4:
      snprintf(s_drug_information,sizeof(s_drug_information),"%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s",
        "Dopamine",
        "Class: Inotrope",
        "Pharmacodynamics: Dopaminergic and beta-1 adrenergic agonist. Has alpha adrenergic activity at high doses.",
        "Shock & CPR",
        "Initiation IV Rate: 2-5 mcg/kg/min.",
        "Maintenance IV Rate: 2-20 mcg/kg/min",
        "Titration Instructions: Assess q10-30 min and adjust 1-4 mcg/kg/min at a time.",
        "Severe Illness",
        "Max IV Rate: 50mcg/kg/min");
    break;
    case 5:
      snprintf(s_drug_information,sizeof(s_drug_information),"%s\n\n%s\n\n%s\n\n%s",
        "Fentanyl",
        "Class: Synthetic Opioid",
        "Pharmacodynamics: µ-opioid receptor agonist.",
        "Dose extremely variable and should be adapted to patient tolerance and requirements.");
    break;
    case 6:
      snprintf(s_drug_information,sizeof(s_drug_information),"%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s",
      	"Heparin",
      	"Class: Anticoagulant",
      	"Pharmacodynamics: Factor Xa antagonist.",
      	"Prophylactic Anticoagulation",
      	"SC Loading Dose: 333u/kg",
      	"SC Maintenance Dose: 250u/kg q12h",
      	"IV Loading Dose: 5000u",
      	"IV Maintenance Rate: 833-1666u/hr");
    break;
    case 7:
      snprintf(s_drug_information,sizeof(s_drug_information),"%s\n\n%s\n\n%s\n\n%s\n\n%s",
      	"Norepinephrine",
      	"Class: Vasopressor",
      	"Pharmacodynamics: alpha-adrenergic receptor agonist. Causes systemic vasoconstriction thereby increasing SVR and BP.",
      	"IV Dose Warning: Dosages can be expressed in terms of free norepinephrine or norepinephrine bitartrate. Below doses are expressed as free norepinephrine (1mg free norepi = 2mg norepi bitartrate).",
      	"mcg/min");
    break;
    case 8:
      strcpy(s_drug_information,"Midazolam\n\nClass: Sedative\n\nPharmacodynamics: Benzodiazepine. GABA receptor agonist.\n\nIV Dose: ");
    break;
  }
  window_set_window_handlers(s_ind_drug_window,(WindowHandlers) {.load=ind_drug_window_load,.unload=ind_drug_window_unload});
  window_stack_push(s_ind_drug_window,true);
}


//Drugs window.
static void drugs_window_load(Window *window) {
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
    .title = "Diltiazem",
    .subtitle = "Cardizem",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Dobutamine",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Dopamine",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Fentanyl",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Heparin",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Levophed",
    .subtitle = "Norepinephrine",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Midazolam",
    .subtitle = "Versed",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Milrinone",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Nitroglycerine",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Nitroprusside",
    .subtitle = "Nipride",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Phenylephrine",
    .subtitle = "Neo-Synephrine",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "PRBCs",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Propofol",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Rocuronium",
    .callback = ind_drug_window_create
  };
  s_drugs_menu_items[drug_menu_count++] = (SimpleMenuItem) {
    .title = "Vasopressin",
    .callback = ind_drug_window_create
  };
  
  s_drugs_menu_sections[0] = (SimpleMenuSection) {
    .title = NULL,
    .num_items = DRUGS_NUM_ITEMS,
    .items = s_drugs_menu_items
  };
  
  s_drugs_menu_layer = simple_menu_layer_create(layer_get_frame(window_get_root_layer(window)),window,s_drugs_menu_sections,DRUGS_NUM_SECTIONS,NULL);
  layer_add_child(window_get_root_layer(window),simple_menu_layer_get_layer(s_drugs_menu_layer));
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

//Code Window handlers.
//!!!! Must set click config provider.

static void code_window_load(Window *window) {
  //Subscribe to the special tick timer service for resuscitation (to make the timer work).
  tick_timer_service_subscribe(SECOND_UNIT,resus_tick_handler);
  
  window_set_background_color(window,GColorBlack);
  
  window_set_click_config_provider(s_code_window,resus_window_click_config_provider);

  //Resuscitation screen title.
  s_resus_title_text_layer = text_layer_create(GRect(0,0,124,20));
  text_layer_set_text(s_resus_title_text_layer,"Resuscitation Mode");
  text_layer_set_text_alignment(s_resus_title_text_layer,GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_resus_title_text_layer));
  
  //Resuscitation timer.
  s_resus_timer_text_layer = text_layer_create(GRect(0,26,60,20));
  text_layer_set_text(s_resus_timer_text_layer,"88:88");
  text_layer_set_text_alignment(s_resus_timer_text_layer,GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_resus_timer_text_layer));
  
  //Resuscitation real-time clock.
  s_resus_clock_text_layer = text_layer_create(GRect(64,26,60,20));
  text_layer_set_text(s_resus_clock_text_layer,"88:88:88");
  text_layer_set_text_alignment(s_resus_clock_text_layer,GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_resus_clock_text_layer));
  
  //Resuscitation shock icon. In future, this could be a lightning bolt icon rather than the letter S.
  s_resus_shock_icon_text_layer = text_layer_create(GRect(128,0,16,55));
  text_layer_set_text(s_resus_shock_icon_text_layer,"S\nh\nk");
  text_layer_set_text_alignment(s_resus_shock_icon_text_layer,GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_resus_shock_icon_text_layer));
  
  //Resuscitation CPR icon.
  s_resus_cpr_icon_text_layer = text_layer_create(GRect(128,59,16,50));
  text_layer_set_text(s_resus_cpr_icon_text_layer,"C\nP\nR");
  text_layer_set_text_alignment(s_resus_cpr_icon_text_layer,GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_resus_cpr_icon_text_layer));
  
  //Resuscitation drug icon. As above, this could be replaced with a graphic.
  s_resus_drug_icon_text_layer = text_layer_create(GRect(128,113,15,55));
  text_layer_set_text(s_resus_drug_icon_text_layer,"D\nr\ng");
  text_layer_set_text_alignment(s_resus_drug_icon_text_layer,GTextAlignmentCenter);
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_resus_drug_icon_text_layer));
  
  //Resuscitation TextLayer.
  s_resus_log_text_layer = text_layer_create(GRect(0,52,123,116));
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_resus_log_text_layer));

  resus_update_time();
}

static void code_window_unload(Window *window) {
  text_layer_destroy(s_resus_title_text_layer);
  window_destroy(window);
  s_resus_timer_seconds = 0;
  tick_timer_service_subscribe(SECOND_UNIT,tick_handler);
  strcpy(s_resus_log,"");
}

//Battery handler.
static void battery_handler(BatteryChargeState charge_state) {
  static char s_battery_buffer[5];
  
  if(charge_state.is_charging) {
    snprintf(s_battery_buffer,sizeof(s_battery_buffer),"C%d",charge_state.charge_percent);
  } else {
    snprintf(s_battery_buffer,sizeof(s_battery_buffer),"%d%%",charge_state.charge_percent);
  }
  
  text_layer_set_text(s_main_window_battery_layer,s_battery_buffer);
  
  //Update battery bar width.
  s_battery_bar_width = ((float)charge_state.charge_percent/100.0)*(float)s_battery_bar_max_width;
  text_layer_set_size(s_main_window_battery_bar_layer,GSize(s_battery_bar_width,10));
}

//Main window click handlers.
static void main_window_up_click_handler(ClickRecognizerRef recognizer,void *context) {
  //Common medications.
  s_drugs_window = window_create();
  window_set_window_handlers(s_drugs_window,(WindowHandlers) {.load=drugs_window_load,.unload=drugs_window_unload});
  window_stack_push(s_drugs_window,true);
}

static void main_window_down_click_handler(ClickRecognizerRef recognizer,void *context) {
  //Common lab values.
  s_labs_window = window_create();
  window_set_window_handlers(s_labs_window,(WindowHandlers) {.load=labs_window_load,.unload=labs_window_unload});
  window_stack_push(s_labs_window,true);
}

static void main_window_select_long_click_handler(ClickRecognizerRef recognizer,void *context) {
  s_code_window = window_create();
  //This is another fullscreen set declaration that must be commented out for compilation with the SDK CLI.
  //window_set_fullscreen(s_code_window,true);
  window_set_window_handlers(s_code_window,(WindowHandlers) {.load=code_window_load,.unload=code_window_unload});
  window_stack_push(s_code_window,true);
}

static void main_window_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP,main_window_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN,main_window_down_click_handler);
  window_long_click_subscribe(BUTTON_ID_SELECT,0,main_window_select_long_click_handler,NULL);
}

//Main window handler.

static void main_window_load(Window *window) {
  window_set_background_color(window,GColorBlack);
  window_set_click_config_provider(s_main_window,main_window_click_config_provider);
  //Create TextLayers.
  s_main_window_params_layer = text_layer_create(GRect(0,0,144,20));
  s_main_window_time_layer = text_layer_create(GRect(0,35,114,50));
  s_main_window_seconds_layer = text_layer_create(GRect(120,50,24,35));
  s_main_window_day_layer = text_layer_create(GRect(0,91,30,20));
  s_main_window_date_layer = text_layer_create(GRect(36,91,22,20));
  s_main_window_month_layer = text_layer_create(GRect(64,91,80,20));
  s_main_window_battery_layer = text_layer_create(GRect(113,112,31,15));
  s_main_window_battery_bar_layer = text_layer_create(GRect(0,117,s_battery_bar_max_width,10));
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
  //Battery number formatting.
  text_layer_set_background_color(s_main_window_battery_layer,GColorBlack);
  text_layer_set_font(s_main_window_battery_layer,fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_color(s_main_window_battery_layer,GColorWhite);
  text_layer_set_text_alignment(s_main_window_battery_layer,GTextAlignmentRight);
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
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_main_window_battery_layer));
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_main_window_battery_bar_layer));
  layer_add_child(window_get_root_layer(window),text_layer_get_layer(s_main_window_labs_layer));
  
  //Get battery state.
  battery_handler(battery_state_service_peek());
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_main_window_params_layer);
  text_layer_destroy(s_main_window_time_layer);
  text_layer_destroy(s_main_window_seconds_layer);
  text_layer_destroy(s_main_window_day_layer);
  text_layer_destroy(s_main_window_date_layer);
  text_layer_destroy(s_main_window_month_layer);
  text_layer_destroy(s_main_window_battery_layer);
  text_layer_destroy(s_main_window_battery_bar_layer);
  text_layer_destroy(s_main_window_labs_layer);
}

static void init() {
  tick_timer_service_subscribe(SECOND_UNIT,tick_handler);
  battery_state_service_subscribe(battery_handler);
  s_main_window = window_create();
  window_set_window_handlers(s_main_window,(WindowHandlers) {.load=main_window_load,.unload=main_window_unload});
  //Removed this line for compliance with new SDK 3.0. On cloudpebble this isn't necessary to remove.
  //window_set_fullscreen(s_main_window,true);
  //Display main window on screen on launch.
  window_stack_push(s_main_window,true);
  update_time();
}

static void deinit() {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}