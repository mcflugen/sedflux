!-------------------------------------------------------------------------
!   ! Start of the sedflux Gridded Component module
    module SEDFLUX_mod

    use ESMF_Mod
    public SEDFLUX_SetServices
    contains

!   ! Public subroutine which the main program will call to register the
!   ! various user-supplied subroutines which make up this Component.
    subroutine SEDFLUX_SetServices(gcomp, rc)
      type(ESMF_GridComp) :: gcomp
      integer :: rc

       call ESMF_GridCompSetEntryPoint(gcomp, ESMF_SETINIT, my_init, &
                                                     ESMF_SINGLEPHASE, rc)
       call ESMF_GridCompSetEntryPoint(gcomp, ESMF_SETRUN, my_run, &
                                                     ESMF_SINGLEPHASE, rc)
       call ESMF_GridCompSetEntryPoint(gcomp, ESMF_SETFINAL, my_final, &
                                                     ESMF_SINGLEPHASE, rc)
      
    end subroutine SEDFLUX_SetServices
      
!   ! User-written Initialization routine
    subroutine my_init(gcomp, import_state, export_state, externalclock, rc)
      implicit none

      type(ESMF_GridComp) :: gcomp
      type(ESMF_State) :: import_state
      type(ESMF_State) :: export_state
      type(ESMF_Clock) :: externalclock
      integer :: rc

      CALL esmf_sedflux_setup( )
      CALL esmf_sedflux_init( )

      print *, "SEDFLUX initialize routine called"
      rc = ESMF_SUCCESS

    end subroutine my_init

!   ! User-written Run routine
    subroutine my_run(gcomp, import_state, export_state, externalclock, rc)
      type(ESMF_GridComp) :: gcomp
      type(ESMF_State)    :: import_state
      type(ESMF_State)    :: export_state
      type(ESMF_Clock)    :: externalclock
      integer             :: rc

      CALL esmf_sedflux_run( )

      print *, "SEDFLUX run routine called"
      rc = ESMF_SUCCESS

    end subroutine my_run

!   ! User-written Finalization routine
    subroutine my_final(gcomp, import_state, export_state, externalclock, rc)
      type(ESMF_GridComp) :: gcomp
      type(ESMF_State) :: import_state
      type(ESMF_State) :: export_state
      type(ESMF_Clock) :: externalclock
      integer :: rc

      CALL esmf_sedflux_finalize( )

      print *, "SEDFLUX finalize routine called"
      rc = ESMF_SUCCESS

    end subroutine my_final

    end module
!   ! End of Second Gridded Component module
!-------------------------------------------------------------------------

