------------------------------------------------------------------------------
--                                                                          --
--                           GNAT ncurses Binding                           --
--                                                                          --
--          Terminal_Interface.Curses.Forms.Field_Types.IPV4_Address        --
--                                                                          --
--                                 B O D Y                                  --
--                                                                          --
--  Version 00.93                                                           --
--                                                                          --
--  The ncurses Ada95 binding is copyrighted 1996 by                        --
--  Juergen Pfeifer, Email: Juergen.Pfeifer@T-Online.de                     --
--                                                                          --
--  Permission is hereby granted to reproduce and distribute this           --
--  binding by any means and for any fee, whether alone or as part          --
--  of a larger distribution, in source or in binary form, PROVIDED         --
--  this notice is included with any such distribution, and is not          --
--  removed from any of its header files. Mention of ncurses and the        --
--  author of this binding in any applications linked with it is            --
--  highly appreciated.                                                     --
--                                                                          --
--  This binding comes AS IS with no warranty, implied or expressed.        --
------------------------------------------------------------------------------
--  Version Control:
--  $Revision: 1.1 $
------------------------------------------------------------------------------
with Interfaces.C;
with Terminal_Interface.Curses.Aux; use Terminal_Interface.Curses.Aux;

package body Terminal_Interface.Curses.Forms.Field_Types.IPV4_Address is

   use type Interfaces.C.Int;

   procedure Set_Field_Type (Fld : in Field;
                             Typ : in Internet_V4_Address_Field)
   is
      C_IPV4_Field_Type : C_Field_Type;
      pragma Import (C, C_IPV4_Field_Type, "TYPE_IPV4");

      function Set_Fld_Type (F    : Field := Fld;
                             Cft  : C_Field_Type := C_IPV4_Field_Type)
                             return C_Int;
      pragma Import (C, Set_Fld_Type, "set_field_type");

      Res : Eti_Error;
   begin
      Res := Set_Fld_Type;
      if Res /= E_Ok then
         Eti_Exception (Res);
      end if;
      Wrap_Builtin (Fld, Typ);
   end Set_Field_Type;

end Terminal_Interface.Curses.Forms.Field_Types.IPV4_Address;