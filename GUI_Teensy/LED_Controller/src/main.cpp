#include "global.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#define VOLTAGE_DISPLAY_UPDATE_MS 100

//********************************************************************
// to connect widgets with code
//********************************************************************
void ObtainGuiWidgets(GtkBuilder *p_builder)
{
  #define GuiappGET(xx) gui_app->xx=GTK_WIDGET(gtk_builder_get_object(p_builder,#xx))
  GuiappGET(window1);
  GuiappGET(entry_sd);
  GuiappGET(label_voltage);
  GuiappGET(scale_red);
  GuiappGET(scale_green);
  GuiappGET(scale_blue);
  GuiappGET(text_red);
  GuiappGET(text_green);
  GuiappGET(text_blue);
  GuiappGET(button_opendevice);
  GuiappGET(button_closedevice);
  GuiappGET(txString);
}


//********************************************************************
// GUI handlers
//********************************************************************
gboolean  Voltage_Display_Displayer(gpointer p_gptr)
{
  // do not change this function
  g_mutex_lock(mutex_to_protect_voltage_display);
  gtk_label_set_text(GTK_LABEL(gui_app->label_voltage),c_voltage_value);
  g_mutex_unlock(mutex_to_protect_voltage_display);
  return true;
}

extern "C" void button_opendevice_clicked(GtkWidget *p_wdgt, gpointer p_data ) 
{
  //do not change  the next few lines
  //they contain the mambo-jumbo to open a serial port
  
  const char *t_device_value;
  struct termios my_serial;

  t_device_value = gtk_entry_get_text(GTK_ENTRY(gui_app->entry_sd));
  //open serial port with read and write, no controling terminal (we don't
  //want to get killed if serial sends CTRL-C), non-blocking 
  ser_dev = open(t_device_value, O_RDWR | O_NOCTTY );
  
  bzero(&my_serial, sizeof(my_serial)); // clear struct for new port settings 
        
  //B9600: set baud rate to 9600
  //   CS8     : 8n1 (8bit,no parity,1 stopbit)
  //   CLOCAL  : local connection, no modem contol
  //   CREAD   : enable receiving characters  */
  my_serial.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
         
  tcflush(ser_dev, TCIFLUSH);
  tcsetattr(ser_dev,TCSANOW,&my_serial);
 
  //You can add code beyond this line but do not change anything above this line
  
  gtk_widget_set_sensitive (gui_app->button_opendevice, FALSE);
  gtk_widget_set_sensitive (gui_app->button_closedevice, TRUE);
  gtk_widget_set_sensitive (gui_app->entry_sd, FALSE);
}

extern "C" void button_closedevice_clicked(GtkWidget *p_wdgt, gpointer p_data ) 
{
  //this is how you disable a button:
  gtk_widget_set_sensitive (gui_app->button_closedevice,FALSE);
  //this is how you enable a button:
  gtk_widget_set_sensitive (gui_app->button_opendevice,TRUE);
  gtk_widget_set_sensitive (gui_app->entry_sd, TRUE);

  //do not change the next two lines; they close the serial port
  close(ser_dev);
  ser_dev=-1;

}

extern "C" void button_send_clicked(GtkWidget *p_wdgt, gpointer p_data ) 
{
  //const char *t_red_value;
  //unsigned char uc_red_value;
  char c_cc_value[40];
  char send_buff[7];
  int length_send_buff = 7;

  const char * rVal= gtk_entry_get_text(GTK_ENTRY(gui_app->text_red));
  const char * gVal= gtk_entry_get_text(GTK_ENTRY(gui_app->text_green));
  const char * bVal= gtk_entry_get_text(GTK_ENTRY(gui_app->text_blue));
  
  int riVal = atoi(rVal);
  int giVal = atoi(gVal);
  int biVal = atoi(bVal);
  
  gtk_range_set_value(GTK_RANGE(gui_app->scale_red), riVal);
  gtk_range_set_value(GTK_RANGE(gui_app->scale_green), giVal);
  gtk_range_set_value(GTK_RANGE(gui_app->scale_blue), biVal                                                  );
		      
  double g_red_value = gtk_range_get_value(GTK_RANGE(gui_app->scale_red));
  double g_green_value = gtk_range_get_value(GTK_RANGE(gui_app->scale_green));
  double g_blue_value = gtk_range_get_value(GTK_RANGE(gui_app->scale_blue));

  char str[40];
  char buff[7];
  int bufLen = 7;
  
  buff[0] = 0xAA;
  buff[1] = 0x07;
  buff[2] = 0x4C;
  buff[3] = (unsigned char) g_red_value;
  buff[4] = (unsigned char) g_green_value;
  buff[5] = (unsigned char) g_blue_value;
  buff[6] = buff[0]^buff[1]^buff[2]^buff[3]^buff[4]^buff[5];
  
  sprintf(str, "%02X%02X%02X%02X%02X%02X%02X", buff[0], buff[1], buff[2], buff[3], buff[4], buff[5], buff[6]);
  gtk_label_set_text(GTK_LABEL(gui_app->txString), str);
  
  write(ser_dev, buff, bufLen);
}

extern "C" void scale_rgb_value_changed(GtkWidget *p_wdgt, gpointer p_data ) 
{
  //getting the value of the scale slider 
  double g_red_value = gtk_range_get_value(GTK_RANGE(gui_app->scale_red));
  double g_green_value = gtk_range_get_value(GTK_RANGE(gui_app->scale_green));
  double g_blue_value = gtk_range_get_value(GTK_RANGE(gui_app->scale_blue));

  //setting text on entry
  char c_cc_value[500];
  
  sprintf(c_cc_value,"%d", (int)g_red_value);
  gtk_entry_set_text(GTK_ENTRY(gui_app->text_red), c_cc_value);
  
  sprintf(c_cc_value, "%d", (int)g_green_value);
  gtk_entry_set_text(GTK_ENTRY(gui_app->text_green), c_cc_value); 
  
  sprintf(c_cc_value, "%d", (int)g_blue_value);
  gtk_entry_set_text(GTK_ENTRY(gui_app->text_blue), c_cc_value);
  
  char str[40];
  char buff[7];
  int bufLen = 7;
  
  buff[0] = 0xAA;
  buff[1] = 0x07;
  buff[2] = 0x4C;
  buff[3] = (unsigned char) g_red_value;
  buff[4] = (unsigned char) g_green_value;
  buff[5] = (unsigned char) g_blue_value;
  buff[6] = buff[0]^buff[1]^buff[2]^buff[3]^buff[4]^buff[5];
  
  sprintf(str, "%02X%02X%02X%02X%02X%02X%02X", buff[0], buff[1], buff[2], buff[3], buff[4], buff[5], buff[6]);
  gtk_label_set_text(GTK_LABEL(gui_app->txString), str);
  
  write(ser_dev, buff, bufLen);
}


extern "C" void button_exit_clicked(GtkWidget *p_wdgt, gpointer p_data ) 
{
  gtk_main_quit();

}



//********************************************************************
//********************************************************************
// 
//   Main loop
//
//********************************************************************
//********************************************************************

int main(int argc, char **argv)
{

  GtkBuilder *builder;
  GError *err = NULL;

  GThread *read_thread;

  //this is how you allocate a Glib mutex
  g_assert(mutex_to_protect_voltage_display == NULL);
  mutex_to_protect_voltage_display = new GMutex;
  g_mutex_init(mutex_to_protect_voltage_display);

  // this is used to signal all threads to exit
  kill_all_threads=false;
  
  //spawn the serial read thread
  read_thread = g_thread_new(NULL,(GThreadFunc)Serial_Read_Thread,NULL);
  
  // Now we initialize GTK+ 
  gtk_init(&argc, &argv);
  
  //create gtk_instance for visualization
  gui_app = g_slice_new(Gui_Window_AppWidgets);

  //builder
  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "teensy_control.glade", &err);

  
  //error handling
  if(err)
    {
      g_error(err->message);
      g_error_free(err);
      g_slice_free(Gui_Window_AppWidgets, gui_app);
      exit(-1);
    }

  // Obtain widgets that we need
  ObtainGuiWidgets(builder);

  // Connect signals
  gtk_builder_connect_signals(builder, gui_app);

  // Destroy builder now that we created the infrastructure
  g_object_unref(G_OBJECT(builder));

  //display the gui
  gtk_widget_show(GTK_WIDGET(gui_app->window1));

  //this is going to call the Voltage_Display_Displayer function periodically
  gdk_threads_add_timeout(VOLTAGE_DISPLAY_UPDATE_MS,Voltage_Display_Displayer,NULL);

  //the main loop
  gtk_main();

  //signal all threads to die and wait for them (only one child thread)
  kill_all_threads=true;
  g_thread_join(read_thread);
  
  //destroy gui if it still exists
  if(gui_app)
    g_slice_free(Gui_Window_AppWidgets, gui_app);

  return 0;
}
