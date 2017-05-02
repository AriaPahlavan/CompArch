    .ORIG x0200

    .FILL x0000
    .FILL x1200     ;0x0202 -> (x01) ISR
    .FILL x1400     ;0x0204 -> (x02) Page-fault
    .FILL x1A00     ;0x0206 -> (x03) Unaligned
    .FILL x1600     ;0x0208 -> (x04) Protection
    .FILL x1C00     ;0x020A -> (x05) Unknown
    
    .END
