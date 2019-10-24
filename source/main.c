/*******************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         main.c
 *
 *   Revision:       REV 1.6
 *
 *   Author:         Ken Langlais, edits by Dave Paquette
 *
 *                   @Copyright 2009, 2015  Scully Signal Company
 *
 *  Description:  Main program loop for Rack controller main microprocessor
 *                PIC24HJ256GP210
 *
 * Revision History:
 *   Rev      Date    Who  Description of Change Made
 *  ------- --------  ---  --------------------------------------------
 *  1.5     09/23/08  KLL  Ported Motorola code to the PIC24HJ256GP210
 *  1.5.23  06/04/12  KLL  Added the displaying of the error registers when the 
 *                          service led is blinking.
 *                         In log_dome_out() made some clarification to the
 *                          sensors that are being logged with the dome out.
 *                         Also changed the log from nvLogRepeat() to nvLogPut().
 *                         Otherwise if multiple trucks have dome outs only one
 *                          would be logged; the others would increment the
 *                          repeat count.
 *  1.5.25  04/09/12  KLL  Added a command that will tell the backup processor
 *                         to continue the overfill bypass after the deadman
 *                         switch is closed.
 *  1.5.26  09/10/12  KLL  Added security.
 *  1.5.27  04/21/14  DHP  Deleted code lines that were commented out.
 *                         Added informational print lines in program load.
 *  1.5.28  05/13/14  DHP  Changed OpenI2C2() call to use variables rather than 
 *                          absolute values.
 *                         Now look at return of init_I_O_Expander() and force
 *                          watchdog reset if continuous error.
 *                         Removed calls to OpenI2Cx prior to reset as this is
 *                          done in reset.
 * 1.5.29  06/24/14  DHP  Much of the downloader code in main was restructured
 *                         to aid debug.
 * 1.5.30  08/10/14  DHP  Added unit_type initialization and call
 *                         to set_ground_reference().
 *                        Removed iambroke vapor check.
 * 1.6.31  01/29/15  DHP  Split apart from loader.
 * 1.6.34  10/14/16  DHP  Fogbugz 141: Pulled backup processor communications out
 *                         of main() and doEighths() into new send_bk_msg(), 
 *                         service_bk_msg() and check_bk() with calls to these
 *                         replacing the code.  within the routines the handling
 *                         was changed to allow more time for responses and fewer
 *                         retries before error.  Lost communication and backup
 *                         reported errors became fatal errors.
 *                        Fogbugz 131, 142: Changed eeinit() return handling to
 *                         treat CRC errors as fatal via call to newly created 
 *                         crc_death().
 *                         Restructured status checking in check_status() and
 *                         added call to crc_death() for EEPROM errors.
 *                         Added logEEPROM() to record the EEPROM errors.
 *                        Restored the SysParm.ADCTmaxNV value to original.
 * 1.6.35  01/07/17  DHP  Increased the wait time in check_bk() due to a missed
 *                         message when a 5-wire vehicle was connected.
 ******************************************************************************/
#include "common.h"
#include "evlog.h"
#include "version.h"
#include "diag.h"
/* comment UPDATER define out if debugging application;no loader will result in invalid
     loader error and cause Optic Pulse LEDs to blink */
#define UPDATER
#ifdef UPDATER
#include "loader.h"
#endif
/******************************* 1/26/2010 8:35AM ****************************
 * The lint -19 is needed to keep lint from complaining that the _F directives
 * from flagging them as Useless Declaration
 *****************************************************************************/
/*lint -e19 */
_FICD((int)ICS_PGD2 & (int)JTAGEN_OFF);
_FOSCSEL((int)FNOSC_PRIPLL);
_FOSC((int)FCKSM_CSDCMD & (int)OSCIOFNC_OFF & (int)POSCMD_XT);
_FWDT((int)FWDTEN_ON & (int)WINDIS_OFF & (int)WDTPRE_PR128 & (int)WDTPOST_PS1024);
_FGS((int)GSS_STD)
/*lint +e19 <= restore lint's ability to flag useless declarations */

typedef enum
{
  BK_OK,
  BK_CRC,
  BK_BAD_MSG,
  BK_HOT_WIRE,
  BK_RELAY,
  BK_CPU,
  BK_PC,
  BK_ROM,
  BK_JUMPER,
  BK_BOOT_MEM,
  BK_MEM
} BK_STATUS;

typedef union
{
  unsigned int reset_save;
  RCONBITS rcon_reg;
}rcon;

static rcon rcon_last __attribute__ ((address(0x3F02)))__attribute__ ((noload));

static void logreset(RCONBITS rcon_reg);
static int msg_time = 0;       /* time count at message send */
static int msg_wait = 0;       /* 1/8 Seconds remaining before fault */

static void log_err_reset(void);
static const BK_ERROR *bk_error;
static int send_bk_status;
static int bk_retry;
static void clrinfo(char *info);
static void doEighths(void);
static void doSeconds(void);
static char lastEighths;        /* Local 125-millisecond trigger */
static void check_status(void);
static void doAbuse(void);
static void logimpact(void);
static void send_bk_msg (unsigned char BK_MSG);
static BK_STATUS service_bk_msg(void);
void check_bk(void);

void  toggle_led(void)
{
  if (MSTAT_LED)
  {
    MSTAT_LED = 0;
  }
  else
  {
    MSTAT_LED = 1;
  }
}
//>>> FogBugz 131, 142
static void crc_death (void)
{
  xprintf(83, (unsigned int)EE_status);
  ledstate[COMPARTMENT_7] = FLASH2HZ;
  ledstate[COMPARTMENT_8] = FLASH2HZ;
  report_tank_state();         /* unified output */
  logEEPROM();
  for(;;)
  {
    modbus_execloop_process();      /* Serial (RS-485) Input */
    ClrWdt();                  /* Prevent reset */
  }
}
//<<< FogBugz 131, 142
//>>> FogBugz 141
#define BK_MSG_STATUS   0x10
#define BK_MSG_VERSION  0x12
#define BK_MSG_BYPASS   0x13
#define BK_MSG_RESEND   0x69

static void send_bk_msg (unsigned char BK_MSG)
{
  static unsigned char last_msg = BK_MSG_VERSION;
  int i;
  unsigned char msg_pkt[20];

  for (i=0; i<5; i++)
  {
    msg_pkt[i] = 0;
  }
  msg_pkt[0] = 2;
  if (BK_MSG == BK_MSG_RESEND)
  {
    bk_retry++;
    msg_pkt[1] = last_msg;
  }else
  {
    msg_pkt[1] = BK_MSG;
    last_msg = msg_pkt[1];
  }
  send_bk_status = TRUE;
  receive_bk_status = FALSE;
  send_backup_pkt((unsigned char*)&msg_pkt[0]);  /* Send  */
  msg_time = loopEighths;          /* Initialize timer */
  msg_wait = (3*8);                   /* Set timeout for 3 seconds */
}

static BK_STATUS service_bk_msg(void)
{
  BK_STATUS status;
  
  status = BK_OK;
  receive_bk_status = FALSE;
  send_bk_status = FALSE;
  if ((print_once_msg & BK_ERR_MSG) == BK_ERR_MSG)
  {
    printf("\n\rBackup Processor communication has been re-established\n\r");
  }
  print_once_msg &= ~(BK_ERR_MSG | BK_NO_COMM_MSG);  /* Reset the print flags */
  if (communicate_RX_pkt[communicate_RX_pkt[0]-1] != Dallas_CRC8
          ((unsigned char *) &communicate_RX_pkt[0], communicate_RX_pkt[0] - 1))
  { //report CRC error
    int i;
    printf("\n\rBackup Processor communication packet CRC error\n\r");
    printf("Calculated CRC: 0x%02X, Packet CRC: 0x%02X\n\r",
      Dallas_CRC8 ((unsigned char *) &communicate_RX_pkt[0],
      communicate_RX_pkt[0] - 1), communicate_RX_pkt[communicate_RX_pkt[0]-1]);
    printf("Packet contents:\n\r");
    for ( i=0; i<11; i++)
    {
      printf("%d: 0x%02X\n\r", i, communicate_RX_pkt[i]);
    }
    status = BK_CRC;
  } else //handle packet data
  {
    bk_error = (void *)&communicate_RX_pkt[0];
    if (bk_error->cmd == BK_MSG_STATUS) /* Response from Status Request */
    {
      if (bk_error->Bypass_Hot_Wired != 0)
      {
        if ((print_once_msg & BK_BYPASS_HOT_WIRED_MSG) == 0)
        {
          printf("\n\rBackup Processor detected hot wired bypass key\n\r");
          print_once_msg |= BK_BYPASS_HOT_WIRED_MSG;
        }
        iambroke |= BK_HW_ERR;  /* force a reset on next cycle */
        status = BK_HOT_WIRE;
      } else
      {
        iambroke &= ~BK_HW_ERR;
        print_once_msg &= ~BK_BYPASS_HOT_WIRED_MSG;
      }
      if (bk_error->Relay_Error != 0)
      {
        if ((print_once_msg & BK_RELAY_ERROR_MSG) == 0)
        {
          printf("\n\rBackup Processor detected Relay Error\n\r");
          print_once_msg |= BK_RELAY_ERROR_MSG;
        }
        iambroke |= BK_RELAY_ERR;  /* force a reset on next cycle */
        status = BK_RELAY;
      } else
      {
        iambroke &= ~BK_RELAY_ERR;
        print_once_msg &= ~BK_RELAY_ERROR_MSG;
      }
      if (bk_error->CPU_Error != 0)
      {
        if ((print_once_msg & BK_CPU_ERROR_MSG) == 0)
        {
          printf("\n\rBackup Processor detected CPU Register Error\n\r");
          print_once_msg |= BK_CPU_ERROR_MSG;
        }
        iambroke |= BK_HW_ERR;  /* force a reset on next cycle */
        status = BK_CPU;
      } else
      {
        print_once_msg &= ~BK_CPU_ERROR_MSG;
      }
      if (bk_error->PC_Error != 0)
      {
        if ((print_once_msg & BK_PC_ERROR_MSG) == 0)
        {
          printf("\n\rBackup Processor detected CPU Program Counter Error\n\r");
          print_once_msg |= BK_PC_ERROR_MSG;
        }
        iambroke |= BK_HW_ERR;  /* force a reset on next cycle */
        status = BK_PC;
      } else
      {
        print_once_msg &= ~BK_PC_ERROR_MSG;
      }
      if (bk_error->ROM_Error != 0)
      {
        if ((print_once_msg & BK_ROM_ERROR_MSG) == 0)
        {
          printf("\n\rBackup Processor detected ROM CRC Error\n\r");
          print_once_msg |= BK_ROM_ERROR_MSG;
        }
        iambroke |= BK_HW_ERR;  /* force a reset on next cycle */
        status = BK_ROM;
      } else
      {
        print_once_msg &= ~BK_ROM_ERROR_MSG;
      }
      if (bk_error->Jumper_Error != 0)
      {
        if ((print_once_msg & BK_JUMPER_ERROR_MSG) == 0)
        {
          printf("\n\rBackup Processor detected Jumper Error Jumper_Status: 0x%02X\n\r", bk_error->Jumper_Error);
          print_once_msg |= BK_JUMPER_ERROR_MSG;
        }
        iambroke |= BK_JUMPER_CHANGE;  /* force a reset on next cycle, return all jumpers off for now */
        status = BK_JUMPER;
      } else
      {
        print_once_msg &= ~BK_JUMPER_ERROR_MSG;
      }
      if (bk_error->Boot_Memory_Error != 0)
      {
        if ((print_once_msg & BK_BOOT_MEMORY_ERROR_MSG) == 0)
        {
          printf("\n\rBackup Processor detected Boot Memory Error\n\r");
          print_once_msg |= BK_BOOT_MEMORY_ERROR_MSG;
        }
        iambroke |= BK_HW_ERR;  /* force a reset on next cycle */
        status = BK_BOOT_MEM;
      } else
      {
        print_once_msg &= ~BK_BOOT_MEMORY_ERROR_MSG;
      }
      if (bk_error->Memory_Error != 0)
      {
        if ((print_once_msg & BK_MEMORY_ERROR_MSG) == 0)
        {
          printf("\n\rBackup Processor detected Memory Error\n\r");
          print_once_msg |= BK_MEMORY_ERROR_MSG;
        }
        iambroke |= BK_HW_ERR;  /* force a reset on next cycle */
        status = BK_MEM;
      } else
      {
        print_once_msg &= ~BK_MEMORY_ERROR_MSG;
      }
    } else
    if (bk_error->cmd == BK_MSG_VERSION)  /* Response from Version Command */
    {
         printf("\n\r    Backup Processor Rev: %X.%d.%d\n\r",
         communicate_RX_pkt[3] >> 4, communicate_RX_pkt[3] & 0x0F, communicate_RX_pkt[2]);
    } else
    if (bk_error->cmd == BK_MSG_BYPASS)  /* Restart bypass */
    {
      Permit_OK = TRUE;  /* Do this if bypassing and deadman is OK */
    } else
    {
      printf("\n\r    Invalid status received from backup processor\n\r");
      status = BK_BAD_MSG;
    }
  }
  if(status == BK_OK)
  {
    bk_retry = 0;
  }
  return status;
}

/*
 The seemingly excesive time wait for the backup is required.  Not sure why but when a 5-wire first connects with 
 Intellitrol2 it really does take this long for the response.  After connection and for every other test case the time is minimal .
*/
void check_bk(void)
{
  BK_STATUS rx_stat;
    
  if (send_bk_status == TRUE)  /* Has a message been sent? */
  {
    if (msg_time != loopEighths)
    {
       msg_time = loopEighths;
       if(msg_wait) msg_wait--;
    }
    if ((receive_bk_status != TRUE) && !(msg_wait)) /* Has the backup processor responded to last command */
    {
      if ((print_once_msg & BK_ERR_MSG) == 0)
      {
        printf("\n\rBackup Processor is not responding\n\r");
        print_once_msg |= BK_ERR_MSG;
      }
      if (bk_retry <= 10)
      {
        send_bk_msg(BK_MSG_RESEND);
      }else
      {
        if ((print_once_msg & BK_NO_COMM_MSG) == 0)
        {
          printf("\n\rShutting down communication to backup processor. No response after retries\n\r");
          print_once_msg |= BK_NO_COMM_MSG;
          iambroke |= BK_HW_ERR;
        }
      }
    }else if (receive_bk_status == TRUE) // response received
    {
      rx_stat = service_bk_msg();
      if(rx_stat == BK_CRC || rx_stat == BK_BAD_MSG)
      {
        send_bk_msg(BK_MSG_RESEND);
      }
      else
      {
      /*************************************************************************
       * If in overfill bypass mode and the deadman goes from open to close
       * tell the backup processor to resume bypass overfill condition
       ************************************************************************/
        if ((Permit_OK == TRUE) && (StatusA & STSA_BYPASS))
        {
          send_bk_msg(BK_MSG_BYPASS);
        }else
        if (Permit_OK == TRUE)
        {
          send_bk_msg(BK_MSG_STATUS);
        }
      }
    }
  }else // no message sent
  {
    if ((Permit_OK == TRUE) && (StatusA & STSA_BYPASS))
    {
      send_bk_msg(BK_MSG_BYPASS);
    }else
    {
      send_bk_msg(BK_MSG_STATUS);
    }
  }
}
//<<< FogBugz 141
  
int __attribute__((__section__(".user_start"))) main (void)
{
  __C30_UART = 2;   /* Using UART 2 as the serial communication */

  //The settings below set up the oscillator and PLL for 16 MIPS as
  //follows:
  //            Crystal Frequency  * (DIVISOR+2)
  // Fcy =     ---------------------------------
  //              PLLPOST * (PRESCLR+2) * 4

  PLLFBD = 0x00C6;                        /* For 20MHz */
  CLKDIV = 0x0048;

// clock switching to incorporate PLL
  __builtin_write_OSCCONH(0x03);    // Initiate Clock Switch to Primary
                                                             // Oscillator with PLL (NOSC=0b011)
  __builtin_write_OSCCONL(0x01);    // Start clock switching

  while (OSCCONbits.COSC != 0x3) {}  // Wait for Clock switch to occur

// Wait for PLL to lock
  while(OSCCONbits.LOCK!=1)
  { }
#ifndef UPDATER
  error_flag = 0;
  rcon_last.reset_save = RCON;
#endif
#ifdef UPDATER
/*lint -e728 */
/*lint -e754 */
/*lint -e843 */
#endif

  init_ports();                /* Initialize the I/O ports */
  service_charge();       /* Appease watchdog */

  init_led();                   /* Initialize LEDs (not yet displayed) */
  Init_Timer1();            /* Free running timer */
  Init_Timer2();            /* Heartbeat and 1ms timer */
  Init_32bit_Timer();     /* Init 32 bit timer */
  Init_Timer3();
  Init_Timer4();

  service_charge();       /* Appease watchdog */

  /* Initialize ADC  */
  Init_ADC();
  init_variables();
  modbus_init();
  UART1Init();
  UART2Init();
  (void)I2C1_reset();
  (void)I2C2_reset();

  int status;
  status = init_I_O_Expander();
  if (status)
  {
  /* We need to correctly read jumpers before continuing so sit here if hard error 
      until watchdog resets everything */
    printf("\n\rError in initializing I/O expander chip\n\r");
    do
    {
      status = init_I_O_Expander();
      service_charge();       /* Appease watchdog */
    } while(status);
    printf("Continuing after IO expander reset\n\r");
  }

  eeInit();                        /* Initialize for EEPROM/NVRAM access */
//>>> FogBugz 131, 142
  if (EE_status & EE_NEW)          /* If eeInit() found unformatted EEPROM */
  {
    EE_status = 0;
    (void)eeFormat();              /* Format/Verify the EEPROM */
    eeInit();                      /* Initialize */
  }
  else                             //<<< FogBugz 131
  {
    if (EE_status & EE_UPDATE)       /* If eeInit() had an old rev # */
    {
      (void)eeUpdateHome();
      (void)eeUpdateSys();         /* Update System NV block with */
                                   /* new parameters */
      EE_status &= (unsigned char)~EE_UPDATE; /* Clear the problem flag */
    }
    else if (EE_status)            /* Any other errors? */
    {
      crc_death();
    }
  }
//<<< FogBugz 131, 142
  if(SysParm.EnaSftFeatures & ENA_INTELLITROL2)
  {
    unit_type = INTELLITROL2;
  }else       
  {
    unit_type = INTELLITROL_PIC;
  }
  set_ground_reference();
  xprintf( 0, DUMMY );          /* Introduce ourselves if anyone cares */
  service_charge();                  /* Appease watchdog */
  if (rcon_last.rcon_reg.WDTO)
  {
    printf("\n\r\n\r   *** WATCHDOG RESET OCURRED ***\n\r");
    printf("WD_timeout\n\r");
  }

  // last_routine = 0;
  ops_ADC( OFF );               /* shut off ADC timer interrupt (T3)  */

#if 0    /* Needs verification; option updates via updater module not currently supported */
  if ( error_flag == LOAD_ERROR_NONE)
  {
    if (!OPTBDPRES)            /* Jump if option board is not present */
    {
      uReg32 ProgramMemory;

      SPIMPolInit();                /* Initialize the SPI bus */
      SPI_EEPROMInit();             /* Initialize the SPI eeprom interface */
      ProgramMemory.Val32 = SPI_EEPROMReadDeviceID();  /* Fetch SPI memory ID */
      if ((ProgramMemory.Val[2] < 0x13) || (ProgramMemory.Val[2] > 0x14))
      {
        error_flag = LOAD_ERROR_10;
      }

      if ( error_flag == LOAD_ERROR_NONE)
      {
        unsigned char psw[8];

        if (SPI_EEPROMReadByte(L_FEATURES_ADDR, psw, 8))
        {
          error_flag = LOAD_ERROR_11;
        }
        if ((psw[0] != 0xFF) || (psw[1] != 0xFF))
        {
          if (EnaFeatures(psw))
          {
            error_flag = LOAD_ERROR_12;
          }
          /* Write the new Enable-Features mask into EEPROM */
          if (nvSysParmUpdate())        /* Write to EEPROM */
          {
            error_flag = LOAD_ERROR_13;
          }
        }
      }
    }
  }
#endif
  service_charge();             /* Keep Service LED off */
  /* At this point "10" volts should be applied to all {6 | 8} channels
     (jump start is still off courtesy of CRTSI.S...will be turned on
     in diagnostics() call below, and will further always be "driven"
     to proper assertion in the main loop truck idle state.

     But first . . . delay a bit before turning on +20 so that the backup
     processor has a chance to read channels 0 and 1 to ascertain 6 vs 8
     compartment state.  Basically, guarantee backup a minimum of a full
     second coming out of reset with Jump-Start turned off. This is kinda
     kludgey. */

  service_wait (8);             /* 8 * 1/8-second intervals = 1 Sec. */

  /* The following accounts for a program update after NV Ram has been initialized */
  if (SysParm.Five_Wire_Display == 0)
  {
     SysParm.Five_Wire_Display = 5;
     (void)nvSysParmUpdate();
  }

  test_for_new_front_panel();  /* Look for new control panel */

  diagnostics(DIA_CHK_INIT);    /* Run Diagnostics/Calibration for following parameters: */
                                      /* Kernel and Flash(Shell) CRC, Dallas Clock, */
                                      /* Reference, open voltages and 6/8 compartment */
                                      /* jumpers, raw, bias, noise voltages, 10/20 voltage */
                                      /* Ground R/Diode, LED Panel, Enable Jumpers */
  
  show_revision();
  
  MBLINK1 = 1;
  MBLINK2 = 0;
  MBLINK3 = 1;
  Led_map_low_word_0 = 0;
  Led_map_low_word_1 = 0;
  Led_map_low_word_2 = 0;
  start_point = 0;
  if ((ConfigA & CFGA_8COMPARTMENT))
  {
    start_point = 0;
  }else
  {
    start_point = 2;                  /* don't test first two probes */
  }
  /* Must wait to eeFormat() until after system clock init'ed by call to
     diagnostics() above (as well as ASCII xprintf...) */

  service_charge();             /* Keep Service LED off */
  eeReport();                         /* Report EEPROM status if ASCII */
  logreset(rcon_last.rcon_reg);             /* Log this system reset/restart */

  if (iambroke & VOLTS_FAULT)         /* Any voltage problems detected? */
  {
    logvolterr();                     /* Yes, Log it in Event Log */
    StatusA |= STSA_FAULT;
    set_nonpermit(LITE);
  }
#ifdef UPDATER
  switch (error_flag)
  {
     case LOAD_ERROR_NONE:
       printf("\n\r\tSPI Update Module not detected\n\r\n\r");
       break;
     case LOAD_ERROR_NOLOAD:
       printf("\n\r\tSPI Update Module not loaded\n\r\n\r");
       break;
     case LOAD_SUCCESS:
       printf("\n\r\tSPI Update Module code successfully loaded\n\r\n\r");
       break;
     default:
       printf("\n\r\tError accessing the SPI Update Module; Error: %02X\n\r\n\r", (int)error_flag);
       ledstate[OPTIC_OUT] = FLASH1HZ;
       ledstate[OPTIC_IN] = FLASH1HZ;
       break;
  }
#endif
  reset_bypass();                     /* Clear all bypass stuff */
  badgndflag |= GND_INIT_TRIAL;

  if (modbus_addr != 0)
  {
     modbus_init();                   /* Initialize ModBus service */
     clear_tx2en();
  }
  lastEighths = loopEighths;          /* Initialize "fast" periodic trigger */

  start_idle = 1;         /* Print probe voltage during idle mode */

/******************************************************************************/
  print_once_msg = 0;
  bk_retry = 0;
  send_bk_msg(BK_MSG_VERSION);        /* Request version */
/*************************** START THE MAIN LOOP *************************
*
* This is the top-level "Main" loop for the Intellitrol.
*
* This loop runs forever (until reset/etc.), and is in essence what passes
* for the master/executive scheduler. It just keeps polling . . .
*
* Nominally, this loop should cycle in the few-tens of milliseconds, if all
* is well. A few "exceptional" cases (shorts testing comes to mind) may
* take substantially longer than that. In general, we should cycle fast
* enough to keep both the Service LED and watchdog happy -- not to mention
* guaranteeing quick response to any overfill/potentially dangerous 
* situations! This should always be less than 30 milliseconds while in active
* ("truck attached and permitting") state, but can be in the 100's of milli-
* seconds if not permitting (shorts tests, for example).
*
* Note: if in "Special Operations" mode, the loop time constraints can be
* more-or-less ignored since by definition we will never permit (won't even
* power the probe/channel lines!). However, it would be considered polite
* to cycle the loop in order to honor ModBus polls (status query, "RESET"
* commands, etc.) and the like.
*
* In *ALL* cases, any code which locks out this main "Loop Level" code
* must keep the watchdog happy, and should also keep the Service LED in the
* appropriate state.
*
**************************************************************************/
  for(;;)
  {
    service_charge();             /* Keep Service LED off */
    if ( U1STAbits.OERR)
    {
      U1STAbits.OERR = 0;       /* Clear the Overflow bit */
    }
    else
    {
      BK_STATUS rx_stat;
      
      // >>> Fogbugz 141
      if ( receive_bk_status == TRUE)  /* Has the backup processor responded to last command */
      {
        rx_stat = service_bk_msg();
        if(rx_stat == BK_CRC || rx_stat == BK_BAD_MSG)
        {
          send_bk_msg(BK_MSG_RESEND);
        }
      }
      // <<< Fogbugz 141
    }
    /* Handle all "periodic" tasks. This is driven by a 125-millisecond
       counter --> eight "ticks" or triggers per second. The slower once-
       per-second timer is, in its turn, driven by doEighths(). */
    if (lastEighths != loopEighths) /* 125-millisecond clock ticked? */
    {                               /* Yes */
      doEighths ();                /* Handle "fast" periodic stuff */
    }

    /* This loop MUST be fast enough to keep the watchdog timer from triggering */
    service_charge();               /* Appease watchdog */
    main_charge();                  /* Drive (toggle) the charge pump */
                                    /*  for the main permit relay */
    permit_relay();                 /* Drive main permit relay as needed */
    read_jumpers(1);                /* Read and verify enable jumpers */
    service_charge();               /* Appease watchdog */

    modbus_execloop_process();      /* Serial (RS-485) Input */
    service_charge();               /* Appease watchdog */
    check_status();                 /* Check FAULT conditions/status */

     /* Drive individual tank/probe LEDs, VIP LEDs, etc. and so forth. I.e update Display*/
    service_charge();               /* Appease watchdog */
    report_tank_state();            /* unified output */
     /* Main dispatch to either truck-servicing operations, or instead to
        drive any "Special" operations in effect. */
    service_charge();               /* Appease watchdog */
    if (StatusB & STSB_SPECIAL)     /* "Normal" or "Special" mode? */
    {
      SpecialOps();                 /* Special -- check special operations */
    }
    else
    {
      service_charge();             /* Appease watchdog */
      main_activity();              /* Normal --  service trucks */
    }
    
  } /* End main Loop Level while/loop forever */
      /* check if time to update LCD with TOD data */
  /*lint -e527 */
  return FALSE;
} /* End of rin0_main() */

/**************************************************************************
* check_status()  --  dynamic operational status/update
*
* check_status() is called in main loop to roam around the assorted status
* bits'n'pieces dynamically calculated by various subsystems (e.g., the
* relay states or the iambroke flags) and set/clear the appropriate StatusA/B
* flags accordingly.
*
* In particular, if anything is truly busted, then check_status() asserts
* the "FAULT" flag.
*
* In a similar vein, if the fault fixes itself, then check_status will be
* pleased and will clear the "FAULT" flag, allowing normal unit operation
* once again. [Consider relay shorting, clearing-of-short...]
*
* This of course means that here is *THE* only place that STSA_FAULT should
* be set or cleared (except implicitly on reset...)!
**************************************************************************/

static void check_status(void)
{
char sts;                   /* Cumulative success/fail status */
static char relay_msg = FALSE;

    sts = 0;                    /* Start out optimistically */

  /* Check on last known relay states: if either the Main or the Backup
     relay is "Broken" (won't close when it should) or "Shorted" (is
     closed and shouldn't be), then flag a Relay Error */

  if ((MainRelaySt | BackRelaySt) & (RELAY_BROKEN | RELAY_SHORTED))
  {
      if(relay_msg == FALSE)
      {
        printf("\n\r MainRelaySt: 0x%04X\n\r", MainRelaySt);
        printf(" BackRelaySt: 0x%04X\n\r", BackRelaySt);
        relay_msg = TRUE;
      }
      StatusB |= STSB_BAD_RELAY;      /* Note relay problems */
  }
  else
  {
      StatusB &= ~STSB_BAD_RELAY;     /* Note relays are OK */
      relay_msg = FALSE;
  }
  /* Check on pending voltage/signal problems. */
  if (iambroke & (RAW_FAULT           /* Raw input voltage error? */
                  | PROBE_BIAS_ERROR  /* 3.5/3.8 bias voltage error? */
                  | NOISE_FAULT       /* Noisy/Erratic channel voltages? */
                  | TOL10V_FAULT      /* Bad 10-volt supply? */
                  | TOL20V_FAULT      /* Bad 20-volt supply? */
                  | TOL5WO_FAULT      /* Bad Optic Pulse (5V) level? */
                 ))
     StatusB |= STSB_ERR_VOLTAGE;     /* Set Fault Bit for bad voltage somewhere */
  else
     StatusB &= ~STSB_ERR_VOLTAGE;     /* All voltages are OK - reset Fault Bit */

  /* Check out the on-board Dallas Time-Of-Day clock status. While accurate
     TOD is nice (for event logging and such), it's not "serious" enough to
     Fault the unit out of service unless we're operating in DateStamp mode
     in which case we *MUST* have a valid clock. */

  if ((pDateStamp)                    /* Do we have a DateStamp block */
      && (pDateStamp->name[0] != 0)   /*  with a specified company ID */
      && (pDateStamp->psw[0] != 0))   /*  AND a specified company password? */
  {                                   /* Yes -- DateStamp is enabled */
     if (clock_status == CLOCK_OK)    /* Is Clock happy? */
        StatusB &= ~STSB_BAD_CLOCK;   /* Yes -- TOD clock running OK */
     else                             /* No -- DateStamp needs clock */
        StatusB |= STSB_BAD_CLOCK;    /* Note problems with TOD clock */
  }

  /* Look around for any complaints serious enough to warrant "FAULT"ing
     the unit and taking it out of service.
     Note that ERR_EEPROM is only set on *SERIOUS* EEPROM error, like it
     is completely unusable (bad home block, etc.), and *not* on merely
     bad byte in TIM array, etc. */
// >>> Fogbugz 142
  if (StatusB & (STSB_ERR_EEPROM  /* Serious EEPROM error? */
                 | STSB_CRC_SHELL     /* Shell CRC failed? */
                 | STSB_CRC_KERNEL    /* Kernel CRC failed? */
                ))
  {
    crc_death();
  }
  if (StatusB & (STSB_ADC_TIMEOUT   /* Problems with A/D converters? */
                 | STSB_BAD_CLOCK     /* Problems with the Dallas TOD clock? */
                 | STSB_ERR_VOLTAGE   /* Voltage problems? */
                 | STSB_GRND_FAULT    /* Bad ground-detect circuit? */
                 | STSB_BAD_RELAY     /* Problems with either relay? */
                ))
  {
    sts = TRUE;                      /* Note "FAULT" condition exists */
  }
// <<< Fogbugz 142
  if (iambroke)                       /* Anything here is a FAULT */
  {
     sts = TRUE;                      /* Note "FAULT" condition exists */
  }
  if (iamsuffering)                   /* Similarly... */
  {
     sts = TRUE;                      /* Note "FAULT" condition exists */
  }
  /* If any serious FAULT conditions exist, then assert the FAULT bit
     (which will in turn force unit non-permissive, etc., yet still leave
     ModBus communications/etc. operational).

     If the DEBUG jumper is in place, then just ignore the Fault condition.
     At this point, all guarantees, such as they are, are null and
     void. This is a big hammer you're swinging! If you hammer your foot,
     it's your own fault. This allows diagnosing a fault condition without
     the annoyance of the unit constantly resetting itself...

     *** Running with the DEBUG jumper is an UNSAFE CONDITION ***  */

  if (StatusA & STSA_DEBUG)
  {
     StatusA &= ~STSA_FAULT;        /* Clear "FAULT" state */
  }
  else
  {
     if (sts)
     {
        StatusA |= STSA_FAULT;      /* Assert "FAULT" state */
     }
     else
     {
        StatusA &= ~STSA_FAULT;     /* Clear "FAULT" state */
     }
  }
} /* End check_status() */

/**************************************************************************
* doAbuse() -- Handle abuse (Impact Sensor triggered) of unit
*
* doAbuse() is called whenever PIT/LED interrupt level has detected that
* the LED/Display board impact sensor has triggered.
*
* Try to "respond" to the abuse by crying out in pain and anguish -- flash
* any LEDs currently marked as DARK (timer_heartbeat() will automatically
* reset them to DARK afterwards).
*
* Log the event into EEPROM with the hope that someone can later associate
* the noted time of the impact with some surly trucker, or can otherwise
* track down some culprit to bill for damages...
**************************************************************************/
static void doAbuse (void)
{
  int i;                        /* Prototypical loop counter */

  // last_routine = 0x3;
  printf("\n\rHad an Abuse hit\n\r");
  if (hitCount > 2)       /* Still counting up (1ms rate)? */
  {                              /* Yes, maybe */
    hitCount = 2;          /* "Normalize" the counter */
    return;                  /* And take action later */
  }
  if (--hitCount)          /* Is the "event" over? */
  {
    return;                  /* Maybe next time... */
  }
    /* The "Impact Event" is over, do what we can about it...log it into
       the system event log */
  logimpact();                /* Log what we can */

  /* Loop over all LEDs, turning all DARK LEDs into LITE LEDs, relying
      on timer_heartbeat() to return them to DARK state afterwards */

  for (i = (int)(PERMIT); i > 0; i--)
  {
    if (ledstate[i] == DARK)        /* Currently OFF? */
    {
      ledstate[i] = PULSE;       /* Yes, Flash this LED ON */
    }
  }
} /* End doAbuse() */

/**************************************************************************
* doEighths() -- Once-Per-Tick (125milliseconds) processing
*
* doEights is called by the 125-millisecond "clock ticking" (actually,
* "having ticked" sometime recently).
*
* The 125-millisecond clock is interrupt driven [see timer_heartbeat()];
* this is the "Loop Level" response to the interrupt event(s).
*
* Specifically, doEighths() (and by extension doSeconds()) is not necessarily
* called exactly every 125 milliseconds, or even 8 times per second
* in any given time period. However, "long term" it will average approximately
* every 125 milliseconds, and will average 8 times per second.
*
* All periodic processing will be invoked the proper number of times (unless
* the system is locked up for more than 32 seconds, which will overflow the
* loopEighths byte-wide clock/counter); if Loop Level is delayed, then this
* code will play catch-up at the rate of one invocation per Loop Level cycle,
* so any code depending on accurate long-term/elapsed time will work properly.
*
* Any code that depends on accurate *intervals* of time should not trigger
* off of doEighths/doSeconds.
**************************************************************************/
static void doEighths (void)
{
  // last_routine = 0x4;
    lastEighths++;                      /* Advance one "tick" */

    /*************************************/
    /* Once-per-Eighth-Second processing */
    /*************************************/

    service_charge();                   /* Similarly, keep Service LED happy */

    /* Check for the LED/display card registering unwarranted acceleration
       (read: "Someone is whomping on us"). Log such events into EEPROM! */

    if (hitCount)                       /* Are we being abused? */
       doAbuse ();                      /* Yes */

    /* Check/set 5-wire-optic "In" and "Out" for VIPER/TAS. As long
       as active_5wire() (or whomever) is pulsing at least once a second
       then VIPER/TAS will be able to see at least one of the two re-
       lated bits latch up, and thus can "report" Optic Out/In states
       fairly consistently. This won't handle the case of "sporadic"
       pulsing, but that shouldn't happen very often... -- Intellitrol
       "should" be either steady 5-wire-optic pulsing, or two-wire,
       or idle/etc.

       Note: there is some window of opportunity here for misreporting
             the pulsing of the Optic/etc "state", but if the Intellitrol
             is operating properly (processing Loop Level fast
             enough), this will "correctly" track the actual code state. */

    if (StatusO & STSO_5WIRE_PULSE1)    /* Have we pulsed recently? */
    {                                   /* Yes */
       StatusO |= STSO_5WIRE_PULSE2;    /* Set "sluggish" bit */
       StatusO &= ~STSO_5WIRE_PULSE1;   /*   and clear "instantaneous" bit */
    }
    else
    {                                  /* No recent pulsing */
       StatusO &= ~STSO_5WIRE_PULSE;   /* So clear all Optic-Out flags */
    }
    if (StatusO & STSO_5WIRE_ECHO1)    /* Seen any Pulse Responses recently? */
    {                                  /* Yes */
       StatusO |= STSO_5WIRE_ECHO2;    /* Set "sluggish" bit */
       StatusO &= ~STSO_5WIRE_ECHO1;   /*   and clear "instantaneous" bit */
    }
    else
    {                                  /* No recent echo/responses */
       StatusO &= ~STSO_5WIRE_ECHO;    /* So clear all Optic-In flags */
    }

    /* Implement a similar hysteresis for "Vehicle Comm" LED report */

    if (StatusO & STSO_COMM_VEHICLE1)  /* Seen any good trucks lately? */
    {                                  /* Yes */
       StatusO |= STSO_COMM_VEHICLE2;  /* Set "sluggish" bit */
       StatusO &= ~STSO_COMM_VEHICLE1; /*   and clear "instantaneous" bit */
    }
    else
    {                                  /* No recent truck communications */
       StatusO &= ~STSO_COMM_VEHICLE;  /* So clear all Vehicle flags */
    }

    if ((lastEighths & 0x03) == 0)  /* Been 4 * 125 milliseconds? */
    {                               /* Yes */
      toggle_led();
    }

    /* End of Eight-times-per-second periodic processing, now trigger the
       Once-Per-Second processing.

       This ensures eight eight-times-per-second calls for each call to
       the once-per-second processing. */

    if ((lastEighths & 0x07) == 0)  /* Been 8 * 125 milliseconds? */
    {                               /* Yes */
      doSeconds();                 /* Invoke once-per-second */
      // >>> Fogbugz 141
      check_bk();
      // <<< Fogbugz 141
    }
} /* End doEighths() */

/**************************************************************************
* doSeconds() -- Once-Per-Second processing
*
* doEights is called by the 125-millisecond clock counting up eight ticks
* (1000 milliseconds or one second).
*
* The 125-millisecond clock is interrupt driven [see timer_heartbeat()];
* this is the "Loop Level" response to the interrupt event(s).
*
* Since doSeconds() is triggered by doEighths(), all timing constraints
* applicable to doEighths (e.g., accuracy of "interval") also apply here.
* In particular, doSeconds() is called "on average" once per second, but
* actual interval is not guaranteed.
**************************************************************************/

static void doSeconds (void)
{
static int secFault = 0;       /* Seconds in fault till RESET */
int loop = 1;

    // last_routine = 0x5;
    /*************************************/
    /**** Once-per-Second processing *****/
    /*************************************/

    present_time++;                              /* Advance system Time-Of-Day */

    /* If we are in a fault state for more than 53 (or so) seconds, then
       force a RESET condition and hope that will clear it up. This should
       result in a one-minute cycling interval (the reset/init sequence takes
       about 7 seconds (to flash the LEDs, etc.), which should give TAS/VIPER
       plenty of time to exchange messages/etc., yet still reset often
       enough that StrayCosmicRays(tm) don't leave the unit faulted out of
       service indefinitely. */

    if (StatusA & STSA_FAULT)           /* System faulted/out of service? */
    {                               /* Yes */
      if (secFault == 0)              /* transition from non-Fault state? */
      {                           /* Yes */
          xprintf( 144, DUMMY);     /* Report error registers */
          secFault = 53;          /* One-minute RESET cycle/interval */
      }
      else
      {
        if (--secFault == 0)      /* Time for a reset? */
        {                         /* Yes */
          secReset = 1;           /* Force immediate board-level RESET */
        }
      }
    }
    else
    {                              /* System is fine (well, no faults) */
      secFault = 0;                /* Clear imminent RESET */
    }

    /* Count down the RESET timer. When it hits zero, we force a board-level
       RESET. This gets the backup as well as us, just like hitting the physi-
       cal RESET switch */

    if (secReset)                 /* RESET queued up? */
    {                             /* Yes... */
      if ( secReset == 1)
      {
        log_err_reset();
        xprintf( 144, DUMMY);     /* Report error registers */
      }
      if (--secReset <= 0)        /* RESET now? */
      {                           /* Yes */
        DelayMS (2000);           /* Should kill us RealSoonNow(tm) */
        COMM_RESET = CLR;         /* Assert RESET (active low) */
        DelayMS (5);              /* Should kill us RealSoonNow(tm) */
        while (loop == 1)         /* Otherwise wait for watchdog */
        {
        }
      }
    }
} /* End doSeconds() */

/**************************************************************************
* clrinfo() -- helper to initialize (clear) the various "info" blocks
**************************************************************************/

static void clrinfo(char * info)  /* Pointer to 22-byte E2LOGREC.Info block */
{
  // last_routine = 0x6;
  memset (info, 0x00, sizeof(EVI_RESET)); /* All EVI_* same size */
}

/**************************************************************************
* logcrcerr -- Create an Event Log entry for Kernel/Shell CRC failure
*
* logcrcerr() is called to create an Event Log entry upon detection of a
* "CRC-16" failure, either in the Kernel or in the Shell. This is "Shell"
* code; if the Kernel detects a CRC error in itself on startup, we should
* never get here! Thus, any "Kernel CRC" failures are those detected by
* the shell after successful system reset/startup. Similarly, the Kernel
* shouldn't start the Shell if it detects a CRC error there, so any errors
* reported here are those detected by the Shell after system reset/startup.
**************************************************************************/

void logcrcerr ()
{
  EVI_HDW_CRC info;
  // last_routine = 0x7;

  printf("\n\r    Logging CRC error: Shell Good: 0x%X  Bad: 0x%X\n\r", Good_Shell_CRC_val, ShellCRCval);
  clrinfo ((char *)&info);            /* Clear out info block */

  info.KernelGood = Good_KernelCRCval;  /* Good Kernel CRC-16 value */
  info.KernelReal = KernelCRCval;       /* Actual Kernel CRC-16 value */
  info.ShellGood = Good_Shell_CRC_val;  /* Good Shell CRC-16 value */
  info.ShellReal = ShellCRCval;         /* Actual Shell CRC-16 value */
  nvLogRepeat (EVHDWERR, EVHDW_CRC, (char *)&info, 18*(unsigned long)60*60);
  return;
} /* End logcrcerr() */

/**************************************************************************
* logEEPROM -- Create an Event Log entry for EEPROM failure
*
* logEEPROM() is called to create an Event Log entry upon detection of a
* EEPROM error.  This is called prior to a system restart.  EE_status
* should indicate the type of failure.
* An arbitrary 15 minute period for repeat logging was used.
**************************************************************************/
void logEEPROM ()
{
  EVI_HDW_EEPROM info;

  printf("\n\r    Logging EEPROM error: EE_status: 0x%X\n\r", EE_status);
  clrinfo ((char *)&info);            /* Clear out info block */

  info.EE_status = EE_status;
  info.StatusA = StatusA;
  info.StatusB = StatusB;
  nvLogRepeat (EVHDWERR, EVHDW_EEPROM, (char *)&info, (unsigned long)15*60);
  return;
} /* End logEEPROM() */

/**************************************************************************
* logimpact -- Create an Event Log entry for Impact Sensor triggered
*
* logimpact() is called to create an Event Log entry when the LED/Display
* board impact sensor is triggered. Note any truck and bypass key info we
* have on hand, in case it can be correlated with who is clobbering the
* unit.
*
* Use a 5-minute interval to distinguish between successive attacks...
**************************************************************************/

static void logimpact ()
{
EVI_IMPACT info;            /* Truck and Key info */

    printf("\n\r    Logging Impact Event\n\r");
  // last_routine = 0x8;
    memcpy (info.Truck, (unsigned char *)&truck_SN[0], BYTESERIAL); /* Copy Truck id, if any */
    memcpy (info.Key, bypass_SN, BYTESERIAL); /* Copy bypass key, if any */
    memset (info.future, 0xFF, sizeof(info.future));

    nvLogRepeat (EVIMPACT, 0, (const char *)&info, (unsigned long)(5 * 60));

    return;

} /* End logimpact() */


/**************************************************************************
* loginit() -- Create an Event Log entry for EEPROM erase
*
* loginit() is called after any EEPROM erase operation (e.g., clear ve-
* hicle list, etc.) to log into EEPROM that such a thing happened (it could
* be someone trying to hide previous activity...who knows...).
**************************************************************************/

void loginit (
    char partid                 /* EEV_* partition mask */
    )
{
EVI_INIT   info;            /* Event log data buffer */
unsigned int *info_ptr;

    printf("\n\r    Logging EEPROM erase and Init\n\r");
    clrinfo ((char *)&info);            /* Clear out info block */
  // last_routine = 0x9;

    /* Fill in the Info buffer with our configuration information */
    info_ptr = (unsigned int*)&info.HardwareVer;
    *info_ptr = int_swap(SHELLVER);

    info_ptr = (unsigned int*)&info.KernelVer;
    *info_ptr = int_swap(SHELLVER);

    info_ptr = (unsigned int*)&info.ShellVer;
    *info_ptr = int_swap(SHELLVER);

    info_ptr = (unsigned int*)&info.Jumpers;
    *info_ptr = int_swap((unsigned int)(((unsigned int)ConfigA << 8) | (unsigned int)enable_jumpers));

    info_ptr = (unsigned int*)&info.Config2;
    *info_ptr = int_swap((unsigned int)(((unsigned int)SysParm.ConfigB << 8) | (unsigned int)SysParm.EnaFeatures));

    nvLogMerge (EVINIT, partid, (char *)&info, (4 * 60 * 60));

} /* End loginit() */

/**************************************************************************
* logrelayerr() -- Create an Event Log entry for Relay Problems
*
* logrelayerr() is called to create an Event Log entry describing problems
* detected with the "main" permit relay or observed with the "backup" relay,
* in particular, if both relays are "shorted".
**************************************************************************/

void logrelayerr ()
{
    EVI_HDW_RELAY info;

    printf("\n\r    Logging Relay Error: Backup: 0x%X    Main: 0x%X\n\r", BackRelaySt, MainRelaySt);
    clrinfo ((char *)&info);            /* Clear out info block */

  // last_routine = 0xA;
    info.BackupSt = BackRelaySt;        /* Log backup relay state */
    info.MainSt = MainRelaySt;          /* Log main relay state */

    nvLogRepeat (EVHDWERR, EVHDW_RELAY, (char *)&info, (unsigned long)(4 * 60 * 60));

    return;

} /* End logrelayerr() */


/**************************************************************************
* logreset() -- Create an Event Log entry for System Reset/Restart.
*
* logreset() is called once after system reset/restart and initialization
* to create a "Reset" Event Log entry.
**************************************************************************/

static void logreset (RCONBITS rcon_reg)
{
EVI_RESET   info;           /* Event log data buffer */
unsigned char rsr = 0;
unsigned int *info_ptr;

    clrinfo ((char *)&info);            /* Clear out info block */

    printf("\n\r    Logging Reset\n\r");
  // last_routine = 0xB;
    /* Fill in the Info buffer with our configuration information */
    info_ptr = (unsigned int*)&info.HardwareVer;
    *info_ptr = int_swap(SHELLVER);

    info_ptr = (unsigned int*)&info.KernelVer;
    *info_ptr = int_swap(SHELLVER);

    info_ptr = (unsigned int*)&info.ShellVer;
    *info_ptr = int_swap(SHELLVER);

    info_ptr = (unsigned int*)&info.Jumpers;
    *info_ptr = int_swap((unsigned int)(((unsigned int)ConfigA << 8) | (unsigned int)enable_jumpers));

    info_ptr = (unsigned int*)&info.Config2;
    *info_ptr = int_swap((unsigned int)(((unsigned int)SysParm.ConfigB << 8) | (unsigned int)SysParm.EnaFeatures));

    if ( rcon_reg.POR)
    {
      rsr |= 0x40;        /* Set Power On Reset bit */
    }
    if ( rcon_reg.EXTR)
    {
      rsr |= 0x80;        /* Set External Reset bit */
    }
    if ( rcon_reg.WDTO)
    {
      rsr |= 0x20;        /* Set Watchdog Reset bit */
    }
    if ( rcon_reg.SWR)
    {
      rsr |= 0x02;        /* Set Reset Instruction bit */
    }
    if ( rcon_reg.BOR)    /* Brown out */
    {
      rsr |= 0x04;        /* Set Loss of Clock bit */
    }
    RCON = 0;                     /* Clear persistence bits */

    /* If we rebooted and the DEBUG jumper is in place, then force a new
       event log entry to ensure that we log the fact we're running with
       the DEBUG jumper (otherwise could reset -- log with no DEBUG, then
       install DEBUG jumper and reset -- would *lose* the DEBUG jumper!) */

    if (ConfigA & CFGA_DEBUG)
        nvLogPut (EVRESET, (char)rsr, (const char *)&info);
    else
        nvLogRepeat (EVRESET, (char)rsr, (char *)&info, (unsigned long)(4 * 60 * 60));

} /* End logreset() */

/**************************************************************************
* logvolterr() -- Create an Event Log entry for Bad Voltages error
*
* logvolterr() can be called "anytime" a voltage problem is detected and
* should be Event Logged. The actual Power On Diagnostics (such as raw_13()
* or probe_tolerance()) do not themselves "log" a problem, they merely de-
* tect the problem. logvolterr() can thus be called after all of the re-
* lated "voltage" diagnostics have run to summarize the results into one
* all-encompassing Event Log entry.
**************************************************************************/

void logvolterr ()
{
EVI_VOLTS volts;            /* Voltage values for log */
char flags;                 /* Error flags */
unsigned int *volt_ptr;
unsigned int *channel_voltage_ptr;
int i;

  // last_routine = 0xC;
    flags = (char)(iambroke & 0xFF);    /* Extract voltage error bits */

    /* Fill in the info block with pertinent info. No need to clear the
       block, as this completely fills it! */

    printf("\n\r    Logging Voltage Error:\n\r");
    printf("Raw13Volt: %u\n\r", Raw13Volt);
    printf("ReferenceVolt: %u\n\r", ReferenceVolt);
    printf("BiasVolt: %u\n\r", BiasVolt);
    volts.Raw13 = int_swap(Raw13Volt);                          /* Save the "raw" voltage */
    volts.RefVolt = int_swap(ReferenceVolt);                    /* Save 1.000 volt */
    volts.BiasVolt = int_swap(BiasVolt);                        /* Save "probe bias voltage */
    channel_voltage_ptr = (unsigned int *)&volts.Ch1;           /* Channel voltage part of the packet */
    if (flags & TOL10V_FAULT)           /* 10V open circuit fault? */
        {                               /* Yes */
        volt_ptr = (unsigned int *)&open_c_volt[0][0];
        printf("TOL10V_FAULT:\n\r");
        }
    else if (flags & TOL20V_FAULT)      /* 20V open circuit fault? */
        {                               /* Yes */
        volt_ptr = (unsigned int *)&open_c_volt[1][0];
        printf("TOL20V_FAULT:\n\r");
        }
    else if (flags & NOISE_FAULT)       /* 10V noise/variance fault? */
        {                               /* Yes */
        volt_ptr = (unsigned int *)&noise_volt[0];
        printf("NOISE_FAULT:\n\r");
        }
    else                                /* "Other" error */
        {                               /* Just log the 8 channels */
        volt_ptr = (unsigned int *)&probe_volt[0];
        }

    /***************************** 11/24/2008 7:44AM *************************
     * Copy voltage into the packet while doing a byte swap
     *************************************************************************/
    for ( i=0; i<8; i++)
    {
      printf("%u ",*volt_ptr);
      *channel_voltage_ptr++ = int_swap(*volt_ptr++);
    }
    printf("\n\r");
    /* Log this voltage problem into EEPROM's Event Log, Merge/Repeating
       on a 4-hour basis */

    nvLogRepeat (EVOLTERR, flags, (char *)&volts, (unsigned long)(4 * 60 * 60));

} /* End logvolterr() */

/**************************************************************************
* logmaintenanceerr() -- Create an Event Log entry for maintenance errors
**************************************************************************/
void logmaintenanceerr()
{
    EVI_MAINTENANCE maintenance;
    maintenance.ch5_high_resistance = 1;    /* Channel 5 resistance is higher than expected calc_tank() */
    
    nvLogRepeat (EVIMAINTENANCE, 1, (char *)&maintenance, (unsigned long)(4 * 60 * 60));
}

/******************************* 10/6/2011 2:49PM ***************************************
 * log_dome_out()
 * This will log a DOME OUT Event. Includes the probe type, all probe states
 * and the truck ID.
 ********************************************************************************************/
void log_dome_out()
{
EEOVF_INFO info;            /* probe type. all probe states, Truck ID */
int i;
int compartment = 0;

  if ( disable_domeout_logging == FALSE)
  {
    for ( i=0; i< 16; i++)          /* Initialize  */
    {
      info.probes_state[i] = (unsigned char)P_UNKNOWN;
    }
//    last_routine = 0x8;
    info.probe = (unsigned char)probe_try_state;
    if ( probe_try_state != OPTIC5)
    {
      if ((ConfigA & CFGA_8COMPARTMENT) == 0)
        compartment = 2;
    }

    for ( i=0; i< (16 - compartment); i++)          /* Copy int array into a character array */
    {
      info.probes_state[i] = (unsigned char)probes_state[i+compartment];
    }
    memcpy (info.Truck, (unsigned char *)&truck_SN[1], 5); /* Copy Truck id, if any, without top byte */
    nvLogPut (EEOVERFILL, 0, (const char *)&info);
  }
} /* End log_dome_out() */

/*********************************************************************************************
 * This will log the unit status prior to a error reset.  All info available is gathered.
 **********************************************************************************************/
static void log_err_reset(void)
{
EE_ERR_RESET_INFO info;            /* state machines states and various status info */

    info.Config = SysParm.EnaFeatures;                        /* Buffer 00 */
    info.StatusA = StatusA;                                           /* Buffer 02 */
    info.StatusB = StatusB;                                           /* Buffer 04 */                                         
    info.Iambroke = iambroke;                                    /* Buffer 06 */      
    info.Iamsuffering = iamsuffering;                          /* Buffer 08 */ 
    info.RefVolt = ReferenceVolt;                                 /* Buffer 10 */    
    info.Ground =  badgndflag;                                   /* Buffer 12 */ 
    info.Main_state = (char)main_state;                      /* Buffer 14 */
    info.Truck_state = (char)truck_state;                    /* Buffer 15 */
    info.Acquire_state = (char)acquire_state;              /* Buffer 16 */
    info.Probe_try_state = (char)probe_try_state;       /* Buffer 17 */
    info.Five_wire_state = (char)optic5_state;             /* Buffer 18 */
    info.Two_wire_state = (char)two_wire_state;        /* Buffer 19 */
    info.B_Relay_state = BackRelaySt;                         /* Buffer 20 */ 
    info.M_Relay_state = MainRelaySt;                        /* Buffer 21 */ 
    nvLogPut (EE_ERR_RESET, 0, (const char *)&info);
}
  /* End Err_Reset() */

