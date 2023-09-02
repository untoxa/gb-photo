        .include "global.s"

        .globl __current_bank

.START_SAVE = 8

        .area _INITIALIZED
_save_rom_bank::
        .ds 1
_save_sram_bank_offset::
        .ds 1
_save_sram_bank_count::
        .ds 1

        .area _INITIALIZER
        .db .START_SAVE
        .db 0
        .db 0

        .area   _CODE_255

.macro .wb addr, val
        ld a, val
        ld (addr), a
.endm

; FLASH write sector routines

; c : sram bank
_HDR_flash_data_routine:
        di

        ldh     a, (__current_bank)
        push    af                              ; save current bank

        ld      a, c

        ld      hl, #_save_rom_bank
        srl     a
        add     (hl)
        ld      (rROMB0), a                     ; switch ROM bank

        ld      h, #0x40                        ; two SRAM banks are saved into one ROM bank.
        bit     0, c
        jr      z, 0$
        set     5, h
0$:
        xor     a
        ld      l, a                            ; destination HL == 0x4000 or HL == 0x6000

        ld      d, #0xA0                        ; source DE == 0xA000
        ld      e, a

        ld      a, (_save_sram_bank_offset)
        add     c
        ld      c, a
1$:
        .wb     #rRAMG, #0x0A                   ; enable SRAM

        .wb     #rRAMB, c                       ; switch SRAM

        ld      a, (de)                         ; read byte
        ld      b, a

        .wb     #rRAMG, #0x00                   ; disable SRAM

        .wb     #0x0555, #0xAA
        .wb     #0x02AA, #0x55
        .wb     #0x0555, #0xA0                  ; perform magic

        ld      a, b
        ld      (hl), a                         ; write byte

        ld      b, #0                           ; wait counter
2$:
        cp      (hl)
        jr      z, 3$                           ; check byte

        push    hl
        pop     hl
        push    hl
        pop     hl                              ; delay 4+3+4+3=14

        dec     b
        jr      nz, 2$

        ld      e, #0                           ; fail
        jr      5$
3$:
        inc     de                              ; next source
        inc     hl                              ; next destination

        ld      a, #0xc0                        ; until de == 0xc000
        cp      d
        jr      nz, 1$

        ld      e, #1                           ; success
5$:
        .wb     #0x4000, #0xF0                  ; reset

        .wb     #rRAMG, #0x0A                   ; enable SRAM back

        pop     af
        ld      (rROMB0), a                     ; restore ROM bank

        xor     a
        ldh     (rIF), a
        ei
        ret
_end_HDR_flash_data_routine:

; c : sram bank
_IG_flash_data_routine:
        di

        ldh     a, (__current_bank)
        push    af                              ; save current bank

        ld      a, c

        ld      hl, #_save_rom_bank
        srl     a
        add     (hl)
        ld      (rROMB0), a                     ; switch ROM bank

        ld      h, #0x40                        ; two SRAM banks are saved into one ROM bank.
        bit     0, c
        jr      z, 0$
        set     5, h
0$:
        xor     a
        ld      l, a                            ; destination HL == 0x4000 or HL == 0x6000

        ld      d, #0xA0                        ; source DE == 0xA000
        ld      e, a

        ld      a, (_save_sram_bank_offset)
        add     c
        ld      c, a
1$:
        .wb     #rRAMG, #0x0A                   ; enable SRAM

        .wb     #rRAMB, c                       ; switch SRAM

        ld      a, (de)                         ; read byte
        ld      b, a

        .wb     #rRAMG, #0x00                   ; disable SRAM

        .wb     #0x0AAA, #0xAA
        .wb     #0x0555, #0x55
        .wb     #0x0AAA, #0xA0                  ; perform magic

        ld      a, b
        ld      (hl), a                         ; write byte

        ld      b, #0                           ; wait counter
2$:
        cp      (hl)
        jr      z, 3$                           ; check byte

        push    hl
        pop     hl
        push    hl
        pop     hl                              ; delay 4+3+4+3=14

        dec     b
        jr      nz, 2$

        ld      e, #0                           ; fail
        jr      5$
3$:
        inc     de                              ; next source
        inc     hl                              ; next destination

        ld      a, #0xc0                        ; until de == 0xc000
        cp      d
        jr      nz, 1$

        ld      e, #1                           ; success
5$:
        .wb     #0x4000, #0xF0                  ; reset

        .wb     #rRAMG, #0x0A                   ; enable SRAM back

        pop     af
        ld      (rROMB0), a                     ; restore ROM bank

        xor     a
        ldh     (rIF), a
        ei
        ret
_end_IG_flash_data_routine:

flash_routine_table:
        ; HDR erase sector routine
        .dw     #_HDR_flash_data_routine
        .dw     #(_HDR_flash_data_routine - _end_HDR_flash_data_routine)
        ; InsideGadgets erase sector routine
        .dw     #_IG_flash_data_routine
        .dw     #(_IG_flash_data_routine - _end_IG_flash_data_routine)

b_save_sram_banks = 255
.globl b_save_sram_banks
_save_sram_banks::
        ldhl    sp, #6
        ld      a, (hl)                         ; a = flash_type

        ldhl    sp, #0
        ld      d, h
        ld      e, l                            ; de = sp

        ld      hl, #flash_routine_table
        add     a
        add     a
        add     l
        ld      l, a
        adc     h
        sub     l
        ld      h, a                            ; hl = &erase_routine_table[flash_type]

        ld      c, (hl)
        inc     hl
        ld      b, (hl)                         ; bc = routine
        inc     hl

        ld      a, (hl+)
        ld      h, (hl)
        ld      l, a                            ; hl = negative offset

        cpl
        inc     a                               ; a = positive length

        add     hl, sp
        ld      sp, hl                          ; allocate ram on stack for the routine

        push    de
        push    hl

        ld      d, b
        ld      e, c
        ld      c, a
        rst     0x30                            ; copy up to 256 bytes in C from DE to HL

        ld      a, (_save_sram_bank_count)
        ld      e, #0
        or      a
        jr      z, 2$
        cp      #9
        jr      nc, 2$
        ld      b, a
        ld      c, c
1$:
        pop     hl
        push    hl                              ; restore routine address into hl

        push    bc
        rst     0x20                            ; call routine
        pop     bc

        ld      a, e
        or      a
        jr      z, 2$

        inc     c
        dec     b
        jr      nz, 1$

2$:
        pop     hl                              ; dispose routine address on stack
        pop     hl
        ld      sp, hl

        ret

; FLASH erase sector routines

_HDR_erase_flash_sector_routine:
        di

        ldh     a, (__current_bank)
        push    af                              ; save current bank

        .wb     #rRAMG, #0x00                   ; disable SRAM

        .wb     #rROMB0, (#_save_rom_bank)

        .wb     #0x4000, #0xF0                  ; reset

        .wb     #0x0555, #0xAA
        .wb     #0x02AA, #0x55
        .wb     #0x0555, #0x80
        .wb     #0x0555, #0xAA
        .wb     #0x02AA, #0x55
        .wb     #0x4000, #0x30                  ; perform magic

        ld      de, #0                          ; wait counter
1$:
        ld      a, (#0x4000)
        cp      #0xFF
        jr      z, 2$                           ; check byte

        push    hl
        pop     hl
        push    hl
        pop     hl                              ; delay 4+3+4+3=14

        dec     e
        jr      nz, 1$
        dec     d
        jr      nz, 1$

        ld      e, #0                           ; fail
        jr      3$
2$:
        ld      e, #1                           ; success
3$:
        .wb     #rRAMG, #0x0A                   ; enable SRAM back

        pop     af
        ld      (rROMB0), a                     ; restore bank

        xor     a
        ldh     (rIF), a
        ei
        ret
_end_HDR_erase_flash_sector_routine:

_IG_erase_flash_sector_routine:
        di

        ldh     a, (#__current_bank)
        push    af                              ; save current bank

        .wb     #rRAMG, #0x00                   ; disable SRAM

        .wb     #rROMB0, (#_save_rom_bank)

        .wb     #0x4000, #0xF0                  ; reset

        .wb     #0x0AAA, #0xAA
        .wb     #0x0555, #0x55
        .wb     #0x0AAA, #0x80
        .wb     #0x0AAA, #0xAA
        .wb     #0x0555, #0x55
        .wb     #0x4000, #0x30                  ; perform magic

        ld      de, #0                          ; wait counter
1$:
        ld      a, (#0x4000)
        cp      #0xFF
        jr      z, 2$                           ; check byte

        push    hl
        pop     hl
        push    hl
        pop     hl                              ; delay 4+3+4+3=14

        dec     e
        jr      nz, 1$
        dec     d
        jr      nz, 1$

        ld      e, #0                           ; fail
        jr      3$
2$:
        ld      e, #1                           ; success
3$:
        .wb     #rRAMG, #0x0A                   ; enable SRAM back

        pop     af
        ld      (rROMB0), a                    ; restore bank

        xor     a
        ldh     (rIF), a
        ei
        ret
_end_IG_erase_flash_sector_routine:

erase_routine_table:
        ; HDR erase sector routine
        .dw     #_HDR_erase_flash_sector_routine
        .dw     #(_HDR_erase_flash_sector_routine - _end_HDR_erase_flash_sector_routine)
        ; InsideGadgets erase sector routine
        .dw     #_IG_erase_flash_sector_routine
        .dw     #(_IG_erase_flash_sector_routine - _end_IG_erase_flash_sector_routine)

b_erase_flash = 255
.globl b_erase_flash
_erase_flash::
        ldhl    sp, #6
        ld      a, (hl)                         ; a = flash_type

        ldhl    sp, #0
        ld      d, h
        ld      e, l                            ; de = sp

        ld      hl, #erase_routine_table
        add     a
        add     a
        add     l
        ld      l, a
        adc     h
        sub     l
        ld      h, a                            ; hl = &erase_routine_table[flash_type]

        ld      c, (hl)
        inc     hl
        ld      b, (hl)                         ; bc = routine
        inc     hl

        ld      a, (hl+)
        ld      h, (hl)
        ld      l, a                            ; hl = negative offset

        cpl
        inc     a                               ; a = positive length

        add     hl, sp
        ld      sp, hl                          ; allocate ram on stack for the routine

        push    de
        push    hl

        ld      d, b
        ld      e, c
        ld      c, a
        rst     0x30                            ; copy up to 256 bytes in C from DE to HL

        pop     hl
        rst     0x20                            ; call routine on stack using call hl

        pop     hl
        ld      sp, hl

        ret
