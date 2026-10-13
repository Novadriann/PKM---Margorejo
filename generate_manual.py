import os
import docx
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT, WD_ALIGN_VERTICAL
from docx.oxml import OxmlElement, parse_xml
from docx.oxml.ns import qn, nsdecls

def create_manual():
    doc = docx.Document()
    
    # ---------------- PAGE SETUP ----------------
    for section in doc.sections:
        section.top_margin = Inches(1.0)
        section.bottom_margin = Inches(1.0)
        section.left_margin = Inches(1.0)
        section.right_margin = Inches(1.0)
        section.header.is_linked_to_previous = False
        section.footer.is_linked_to_previous = False
        
        # Footer Setup
        p_ft = section.footer.paragraphs[0]
        p_ft.alignment = WD_ALIGN_PARAGRAPH.RIGHT
        p_ft_run = p_ft.add_run("Buku Panduan Teknis Wiring Sistem NURTURE | PKM Margorejo 2026")
        p_ft_run.font.name = "Calibri"
        p_ft_run.font.size = Pt(8.5)
        p_ft_run.font.color.rgb = RGBColor(120, 120, 120)

    # ---------------- COLORS ----------------
    C_DARK_GREEN = RGBColor(30, 70, 32)      # #1E4620 (Primary UGM Green)
    C_MID_GREEN  = RGBColor(45, 106, 79)     # #2D6A4F
    C_SLATE      = RGBColor(38, 70, 83)      # #264653
    C_CHARCOAL   = RGBColor(40, 40, 40)      # Body text
    C_MUTED      = RGBColor(100, 100, 100)

    HEX_PRIMARY  = "1E4620"
    HEX_HEADER   = "245729"
    HEX_LIGHT_BG = "F4FBF5"
    HEX_ALT_ROW  = "F9FDF9"
    HEX_BORDER   = "D1D5DB"

    img_dir = r"C:\Users\User\.gemini\antigravity\brain\47f3aa70-789e-437d-809b-ba0b6d8a3d67\.user_uploaded"
    img_schematic = os.path.join(img_dir, "media_1791899608847_9eb5c92c.png")
    img_3d_iso    = os.path.join(img_dir, "media_1791899608836_563a3be2.png")
    img_3d_top    = os.path.join(img_dir, "media_1791899608840_1d52ae04.png")
    img_2d_pcb    = os.path.join(img_dir, "media_1791899608830_23f4d8eb.png")

    # ---------------- STYLES ----------------
    styles = doc.styles
    normal_style = styles['Normal']
    normal_style.font.name = 'Calibri'
    normal_style.font.size = Pt(11)
    normal_style.font.color.rgb = C_CHARCOAL
    normal_style.paragraph_format.line_spacing = 1.15
    normal_style.paragraph_format.space_after = Pt(4)

    def add_title(text):
        p = doc.add_paragraph()
        p.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p.paragraph_format.space_before = Pt(12)
        p.paragraph_format.space_after = Pt(2)
        run = p.add_run(text)
        run.bold = True
        run.font.name = 'Arial'
        run.font.size = Pt(20)
        run.font.color.rgb = C_DARK_GREEN
        return p

    def add_h1(text):
        p = doc.add_paragraph()
        p.paragraph_format.space_before = Pt(18)
        p.paragraph_format.space_after = Pt(6)
        p.paragraph_format.keep_with_next = True
        run = p.add_run(text)
        run.bold = True
        run.font.name = 'Arial'
        run.font.size = Pt(14)
        run.font.color.rgb = C_DARK_GREEN
        return p

    def add_h2(text):
        p = doc.add_paragraph()
        p.paragraph_format.space_before = Pt(12)
        p.paragraph_format.space_after = Pt(4)
        p.paragraph_format.keep_with_next = True
        run = p.add_run(text)
        run.bold = True
        run.font.name = 'Arial'
        run.font.size = Pt(12)
        run.font.color.rgb = C_MID_GREEN
        return p

    def add_h3(text):
        p = doc.add_paragraph()
        p.paragraph_format.space_before = Pt(8)
        p.paragraph_format.space_after = Pt(2)
        p.paragraph_format.keep_with_next = True
        run = p.add_run(text)
        run.bold = True
        run.font.name = 'Calibri'
        run.font.size = Pt(11)
        run.font.color.rgb = C_SLATE
        return p

    def add_callout(title, text, box_type="info"):
        b_color = "1E4620" if box_type == "info" else ("D97706" if box_type == "warning" else "DC2626")
        bg_col  = "F0FDF4" if box_type == "info" else ("FFFBEB" if box_type == "warning" else "FEF2F2")
        icon    = "💡 INFORMASI TEKNIS:" if box_type == "info" else ("⚠️ PERINGATAN KESELAMATAN:" if box_type == "warning" else "🚨 CATATAN KHUSUS:")
        
        table = doc.add_table(rows=1, cols=1)
        table.alignment = WD_TABLE_ALIGNMENT.CENTER
        table.autofit = False
        cell = table.cell(0, 0)
        cell.width = Inches(6.5)
        
        # cell shading
        tcPr = cell._tc.get_or_add_tcPr()
        shd = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{bg_col}"/>')
        tcPr.append(shd)
        
        # cell padding
        tcMar = parse_xml(f'<w:tcMar {nsdecls("w")}>'
                          f'<w:top w:w="120" w:type="dxa"/>'
                          f'<w:bottom w:w="120" w:type="dxa"/>'
                          f'<w:left w:w="180" w:type="dxa"/>'
                          f'<w:right w:w="180" w:type="dxa"/>'
                          f'</w:tcMar>')
        tcPr.append(tcMar)
        
        # borders (only left thick border)
        borders = parse_xml(f'<w:tcBorders {nsdecls("w")}>'
                            f'<w:top w:val="none"/>'
                            f'<w:bottom w:val="none"/>'
                            f'<w:left w:val="single" w:sz="24" w:space="0" w:color="{b_color}"/>'
                            f'<w:right w:val="none"/>'
                            f'</w:tcBorders>')
        tcPr.append(borders)
        
        p = cell.paragraphs[0]
        p.paragraph_format.space_before = Pt(0)
        p.paragraph_format.space_after = Pt(2)
        rt = p.add_run(f"{icon} {title}\n")
        rt.bold = True
        rt.font.size = Pt(10)
        rt.font.color.rgb = RGBColor.from_string(b_color)
        
        rb = p.add_run(text)
        rb.font.size = Pt(9.5)
        rb.font.color.rgb = RGBColor(40, 40, 40)
        
        p_sp = doc.add_paragraph()
        p_sp.paragraph_format.space_before = Pt(0)
        p_sp.paragraph_format.space_after = Pt(4)

    def create_custom_table(col_widths, headers, data):
        table = doc.add_table(rows=len(data) + 1, cols=len(headers))
        table.alignment = WD_TABLE_ALIGNMENT.CENTER
        table.autofit = False
        
        # Header row
        hdr_cells = table.rows[0].cells
        for i, h_text in enumerate(headers):
            cell = hdr_cells[i]
            cell.width = Inches(col_widths[i])
            cell.vertical_alignment = WD_ALIGN_VERTICAL.CENTER
            
            tcPr = cell._tc.get_or_add_tcPr()
            shd = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{HEX_HEADER}"/>')
            tcPr.append(shd)
            
            tcMar = parse_xml(f'<w:tcMar {nsdecls("w")}>'
                              f'<w:top w:w="120" w:type="dxa"/>'
                              f'<w:bottom w:w="120" w:type="dxa"/>'
                              f'<w:left w:w="140" w:type="dxa"/>'
                              f'<w:right w:w="140" w:type="dxa"/>'
                              f'</w:tcMar>')
            tcPr.append(tcMar)
            
            borders = parse_xml(f'<w:tcBorders {nsdecls("w")}>'
                                f'<w:top w:val="single" w:sz="6" w:color="{HEX_PRIMARY}"/>'
                                f'<w:bottom w:val="single" w:sz="12" w:color="{HEX_PRIMARY}"/>'
                                f'<w:left w:val="single" w:sz="4" w:color="{HEX_BORDER}"/>'
                                f'<w:right w:val="single" w:sz="4" w:color="{HEX_BORDER}"/>'
                                f'</w:tcBorders>')
            tcPr.append(borders)
            
            p = cell.paragraphs[0]
            p.alignment = WD_ALIGN_PARAGRAPH.CENTER
            p.paragraph_format.space_before = Pt(0)
            p.paragraph_format.space_after = Pt(0)
            run = p.add_run(h_text)
            run.bold = True
            run.font.size = Pt(9.5)
            run.font.color.rgb = RGBColor(255, 255, 255)
            
        # Data rows
        for r_idx, row_data in enumerate(data):
            row = table.rows[r_idx + 1]
            bg = HEX_ALT_ROW if (r_idx % 2 == 1) else "FFFFFF"
            for c_idx, val in enumerate(row_data):
                cell = row.cells[c_idx]
                cell.width = Inches(col_widths[c_idx])
                cell.vertical_alignment = WD_ALIGN_VERTICAL.CENTER
                
                tcPr = cell._tc.get_or_add_tcPr()
                shd = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{bg}"/>')
                tcPr.append(shd)
                
                tcMar = parse_xml(f'<w:tcMar {nsdecls("w")}>'
                                  f'<w:top w:w="90" w:type="dxa"/>'
                                  f'<w:bottom w:w="90" w:type="dxa"/>'
                                  f'<w:left w:w="140" w:type="dxa"/>'
                                  f'<w:right w:w="140" w:type="dxa"/>'
                                  f'</w:tcMar>')
                tcPr.append(tcMar)
                
                borders = parse_xml(f'<w:tcBorders {nsdecls("w")}>'
                                    f'<w:top w:val="single" w:sz="4" w:color="{HEX_BORDER}"/>'
                                    f'<w:bottom w:val="single" w:sz="4" w:color="{HEX_BORDER}"/>'
                                    f'<w:left w:val="single" w:sz="4" w:color="{HEX_BORDER}"/>'
                                    f'<w:right w:val="single" w:sz="4" w:color="{HEX_BORDER}"/>'
                                    f'</w:tcBorders>')
                tcPr.append(borders)
                
                p = cell.paragraphs[0]
                p.paragraph_format.space_before = Pt(0)
                p.paragraph_format.space_after = Pt(0)
                if len(str(val)) <= 6 or c_idx == 0:
                    p.alignment = WD_ALIGN_PARAGRAPH.CENTER
                else:
                    p.alignment = WD_ALIGN_PARAGRAPH.LEFT
                run = p.add_run(str(val))
                run.font.size = Pt(9)
                run.font.color.rgb = C_CHARCOAL
                
        p_after = doc.add_paragraph()
        p_after.paragraph_format.space_before = Pt(0)
        p_after.paragraph_format.space_after = Pt(6)
        return table

    def add_image_box(img_path, caption, width=Inches(6.0)):
        if os.path.exists(img_path):
            p = doc.add_paragraph()
            p.alignment = WD_ALIGN_PARAGRAPH.CENTER
            p.paragraph_format.space_before = Pt(6)
            p.paragraph_format.space_after = Pt(2)
            p.paragraph_format.keep_with_next = True
            doc.add_picture(img_path, width=width)
            
            p_cap = doc.add_paragraph()
            p_cap.alignment = WD_ALIGN_PARAGRAPH.CENTER
            p_cap.paragraph_format.space_before = Pt(2)
            p_cap.paragraph_format.space_after = Pt(8)
            r_cap = p_cap.add_run(caption)
            r_cap.font.size = Pt(9)
            r_cap.font.italic = True
            r_cap.font.color.rgb = C_MUTED

    # =========================================================================
    # 1. COVER / HEADER
    # =========================================================================
    p_inst = doc.add_paragraph()
    p_inst.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_inst.paragraph_format.space_before = Pt(0)
    p_inst.paragraph_format.space_after = Pt(2)
    r_inst = p_inst.add_run("PROGRAM PENGABDIAN KEPADA MASYARAKAT (PKM) 2026\nUNIVERSITAS GADJAH MADA — MARGOREJO, SLEMAN")
    r_inst.bold = True
    r_inst.font.name = "Arial"
    r_inst.font.size = Pt(11)
    r_inst.font.color.rgb = C_MID_GREEN

    add_title("BUKU PANDUAN TEKNIS & WIRING\nSISTEM PERTANIAN CERDAS MANDIRI ENERGI\n\"NURTURE\"")

    p_sub = doc.add_paragraph()
    p_sub.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_sub.paragraph_format.space_before = Pt(4)
    p_sub.paragraph_format.space_after = Pt(16)
    r_sub = p_sub.add_run("Pedoman Operasional, Integrasi Catu Daya Bertingkat, Perkabelan Sensor, dan Perawatan Mandiri untuk Petani & Teknisi Lapangan")
    r_sub.font.italic = True
    r_sub.font.size = Pt(11)
    r_sub.font.color.rgb = C_SLATE

    # Metadata Table
    meta_headers = ["Parameter", "Keterangan"]
    meta_data = [
        ["Nama Sistem", "NURTURE: Smart Agriculture Mandiri Energi Berbasis IoT & Web"],
        ["Lokasi Kegiatan", "Lahan Pertanian Kalurahan Margorejo, Kapanewon Tempel, Sleman, D.I. Yogyakarta"],
        ["Dosen Pembimbing", "Nur Achmad Sulistyo Putro, S.Si., M.Cs., Ph.D."],
        ["Platform Pengendali", "ESP32-S3 DevKitC-1 N16R8V Dual Type-C (Dual-Core 240MHz, Wi-Fi/BLE)"]
    ]
    create_custom_table([1.8, 4.7], meta_headers, meta_data)

    doc.add_page_break()

    # =========================================================================
    # BAB I: PENGENALAN ARSITEKTUR PERANGKAT KERAS
    # =========================================================================
    add_h1("BAB I. PENGENALAN ARSITEKTUR SISTEM & DESAIN PCB BERTINGKAT")
    
    p = doc.add_paragraph()
    p.add_run(
        "Sistem NURTURE dirancang secara khusus untuk bekerja secara mandiri (off-grid) di area persawahan "
        "Margorejo yang minim jangkauan listrik PLN. Agar sistem memiliki daya tahan tinggi terhadap gangguan lingkungan, "
        "interferensi elektromagnetik, dan cuaca ekstrem, perangkat keras pengendali dibangun menggunakan konsep "
    )
    r_bold = p.add_run("PCB Bertingkat (Stack PCB Architecture) 2 Tingkat.")
    r_bold.bold = True

    add_h2("1.1 Filosofi dan Alasan Desain PCB Bertingkat (Dual-Tier)")
    p = doc.add_paragraph()
    p.add_run(
        "Di dalam kotak panel kendali pertanian terdapat dua jenis perangkat dengan karakteristik listrik yang sangat bertolak belakang:\n"
        "1. Perangkat Logika & Sensor Mikro: Membutuhkan tegangan stabil (3.3V dan 5V), arus sangat kecil, dan sensitif terhadap noise listrik.\n"
        "2. Perangkat Daya & Aktuator Beban: Membutuhkan arus besar (tegangan 12V DC), menarik arus hentakan saat menyala (seperti motor kipas, relay, dan solenoid valve), serta menghasilkan tegangan kejut balik (back-EMF).\n\n"
        "Jika kedua kelompok komponen ini diletakkan pada satu keping PCB biasa, hentakan listrik dari pompa/valve akan langsung "
        "mengacaukan pembacaan sensor tanah atau menyebabkan chip mikrokontroler hang/restart (dikenal sebagai fenomena Brownout). "
        "Oleh karena itu, sistem memisahkan fisiknya menjadi dua tingkat:"
    )

    add_callout(
        "Mengapa PCB Dibagi Menjadi Dua Tingkat?",
        "Tingkat Bawah (Lower Tier) dikhususkan sebagai Regulator & Jalur Daya Arus Tinggi (12V, 5V, 3.3V).\n"
        "Tingkat Atas (Upper Tier) dikhususkan sebagai Pusat Pemrosesan Data & Antarmuka Sensor Mikro.\n"
        "Pemisahan ini menjamin kestabilan modul ESP32-S3 dan sensor NPK 7-in-1 dari gangguan motorik aktuator.",
        box_type="info"
    )

    add_h2("1.2 PCB Tingkat Bawah (Lower Tier — Power & Step-Down Board)")
    p = doc.add_paragraph()
    p.add_run(
        "PCB tingkat bawah berfungsi sebagai 'jantung distribusi energi' yang menerima arus listrik DC dari Baterai VRLA 12V 12Ah "
        "yang diisi oleh Panel Surya (PLTS) melalui Solar Charge Controller (SCC). Di tingkat bawah ini terpasang modul-modul penurun tegangan (Step-Down DC-DC):\n"
        "• Jalur Rel 12V DC: Menyalurkan listrik langsung dari baterai ke aktuator berat (Solenoid Valve 12V, Lampu UV Trap 12V, dan Kipas 12V).\n"
        "• Jalur Rel 5V DC (Step-Down 12V ke 5V): Mengubah 12V baterai menjadi 5V teregulasi (kapasitas 3A) untuk mencatu pin 5V ESP32-S3, modul konverter MAX485 RS485, modul koil relay, dan sensor DHT22.\n"
        "• Jalur Rel 3.3V DC (Step-Down 12V ke 3.3V): Menyediakan tegangan kerja murni 3.3V untuk pin logika sensitif.\n"
        "• Titik Common Ground (GND Bersama): Seluruh kutub negatif (GND) dari 12V, 5V, dan 3.3V diikat menjadi satu bidang ground bersama (star ground) agar referensi tegangan tidak melayang."
    )

    add_h2("1.3 PCB Tingkat Atas (Upper Tier — Controller & Sensor Interface)")
    p = doc.add_paragraph()
    p.add_run(
        "PCB tingkat atas berfungsi sebagai 'otak dan pusat saraf' pengendali otomatisasi. Komponen-komponen utama yang terpasang di PCB atas meliputi:\n"
        "• Soket Mikrokontroler ESP32-S3 DevKitC-1 N16R8V (16MB Flash, 8MB PSRAM).\n"
        "• IC / Modul Transceiver RS485 (U2 - MAX485) untuk membaca protokol industri Modbus RTU pada sensor NPK 7-in-1.\n"
        "• Tiga Unit Driver Transistor NPN (Q1, Q2, Q3) beserta resistor basis 2kΩ (R1, R2, R3) sebagai sakelar penyala relay.\n"
        "• Deretan Soket Terminal Sensor: JST-XH dan Screw Terminal untuk Sensor NPK, 3x Probe Kelembapan Tanah, 3x Sensor DHT22, Sensor Cahaya LDR, dan RTC DS3231.\n"
        "• Soket Interkoneksi Daya Vertikal: Menerima rel 12V, 5V, dan 3.3V dari PCB tingkat bawah."
    )

    add_image_box(img_3d_iso, "Gambar 1.1: Visualisasi 3D PCB Tingkat Atas (Main Controller & Sensor Interface Board)", width=Inches(5.8))

    doc.add_page_break()

    # =========================================================================
    # BAB II: PANDUAN LENGKAP WIRING PCB TINGKAT ATAS
    # =========================================================================
    add_h1("BAB II. PANDUAN WIRING & TERMINAL KONEKTOR PCB ATAS")

    p = doc.add_paragraph()
    p.add_run(
        "Bab ini memandu secara rinci letak setiap konektor, nomor pin, warna kabel standar, serta fungsi sambungannya ke perangkat luar. "
        "Pastikan teknisi lapangan atau kelompok tani membaca tabel ini sebelum mencolokkan kabel sensor atau beban."
    )

    add_image_box(img_3d_top, "Gambar 2.1: Tata Letak Konektor dan Silkscreen PCB Tingkat Atas (Tampak Atas)", width=Inches(5.8))

    add_h2("2.1 Tabel Konektor Interkoneksi Daya (Dari PCB Bawah ke Atas)")
    p = doc.add_paragraph()
    p.add_run("Tegangan kerja didistribusikan dari PCB tingkat bawah melalui dua blok konektor vertikal:")

    hdr_pwr = ["Nama Konektor", "Pin", "Nama Sinyal", "Tegangan", "Keterangan Fungsi"]
    data_pwr = [
        ["J_12V_FROM_BOTTOM1", "Pin 1", "+12V", "+12 Volt DC", "Masukan daya utama dari Baterai PLTS (tingkat bawah)"],
        ["J_12V_FROM_BOTTOM1", "Pin 2", "GND", "0 Volt (Ground)", "Kutub negatif / Ground daya utama"],
        ["J_5v_3v3", "Pin 1", "+5V", "+5.0 Volt DC", "Keluaran Step-Down 5V dari tingkat bawah (untuk ESP & Sensor)"],
        ["J_5v_3v3", "Pin 2", "+3V3", "+3.3 Volt DC", "Keluaran Step-Down 3.3V dari tingkat bawah"],
        ["J_5v_3v3", "Pin 3", "GND", "0 Volt (Ground)", "Jalur Ground logika bersama"],
        ["J_5v_3v3", "Pin 4", "GND", "0 Volt (Ground)", "Jalur Ground logika bersama (paralel)"]
    ]
    create_custom_table([1.5, 0.6, 1.0, 1.1, 2.3], hdr_pwr, data_pwr)

    add_h2("2.2 Tabel Konektor Sensor Tanah, Nutrisi, dan Lingkungan")
    p = doc.add_paragraph()
    p.add_run("Seluruh sensor pertanian dihubungkan ke deretan konektor putih di sisi bawah dan kanan PCB:")

    hdr_sens = ["Kode Soket", "Pin", "Nama Jalur", "GPIO ESP32", "Keterangan & Warna Kabel Standar"]
    data_sens = [
        ["J_NPK1", "Pin 1", "VCC (+5V/12V)", "—", "Catu daya sensor NPK 7in1 (Kabel COKELAT)"],
        ["J_NPK1", "Pin 2", "GND", "—", "Kutub negatif sensor NPK (Kabel HITAM)"],
        ["J_NPK1", "Pin 3", "RS485 A (+)", "Via IC U2", "Data Modbus Positif / Non-Inverting (Kabel KUNING)"],
        ["J_NPK1", "Pin 4", "RS485 B (-)", "Via IC U2", "Data Modbus Negatif / Inverting (Kabel BIRU)"],
        ["J_LEMBAB1", "1, 2, 3", "VCC, OUT, GND", "GPIO 10 (ADC)", "Probe Kelembapan Tanah Analog 1 (Kapasitif)"],
        ["J_LEMBAB2", "1, 2, 3", "VCC, OUT, GND", "GPIO 11 (ADC)", "Probe Kelembapan Tanah Analog 2 (Kapasitif)"],
        ["J_LEMBAB3", "1, 2, 3", "VCC, OUT, GND", "GPIO 12 (ADC)", "Probe Kelembapan Tanah Analog 3 / Cadangan"],
        ["J_DHT_BOX1", "1, 2, 3", "VCC, DATA, GND", "GPIO 7", "Sensor DHT22 Suhu & Kelembapan Kotak Panel"],
        ["J_DHT_OUT1", "1, 2, 3", "VCC, DATA, GND", "GPIO 6", "Sensor DHT22 Cuaca Lingkungan Luar 1"],
        ["J_DHT_OUT2", "1, 2, 3", "VCC, DATA, GND", "GPIO 16", "Sensor DHT22 Cuaca Lingkungan Luar 2"],
        ["J_LDR1", "1, 2, 3, 4", "VCC, OUT, GND", "GPIO 4 (ADC)", "Sensor Cahaya Matahari LDR (Intensitas Radiasi)"],
        ["J_RTC1", "1, 2", "VCC, GND", "—", "Catu daya Modul DS3231 Waktu Presisi (5V)"],
        ["J_RTC1", "Pin 3", "SDA (Data I2C)", "GPIO 8", "Jalur data I2C RTC DS3231"],
        ["J_RTC1", "Pin 4", "SCL (Clock I2C)", "GPIO 9", "Jalur clock I2C RTC DS3231"]
    ]
    create_custom_table([1.1, 0.6, 1.1, 1.1, 2.6], hdr_sens, data_sens)

    add_callout(
        "Kabel Modbus RS485 Sensor NPK 7-in-1",
        "Warna kabel standar pabrik sensor NPK 7in1 adalah:\n"
        "• Cokelat: VCC (+5V / +12V)\n"
        "• Hitam: GND (Ground)\n"
        "• Kuning: Pin A (RS485+)\n"
        "• Biru: Pin B (RS485-)\n"
        "JANGAN SAMPAI TERBALIK antara kabel A (Kuning) dan B (Biru) karena akan menyebabkan kode error pembacaan NPK di layar monitor.",
        box_type="warning"
    )

    doc.add_page_break()

    add_h2("2.3 Rangkaian Penggerak Aktuator (Driver Transistor & Relay)")
    p = doc.add_paragraph()
    p.add_run(
        "Untuk melindungi ESP32-S3 dari tarikan arus koil relay dan lonjakan tegangan, PCB atas dilengkapi sirkuit sakelar transistor NPN (BJT). "
        "Pin GPIO mikrokontroler tidak menyalakan relay secara langsung, melainkan mengalirkan arus kecil melalui resistor 2kΩ ke basis transistor, "
        "yang kemudian men-trigger jalur relay eksternal."
    )

    hdr_act = ["Aktuator Beban", "Pin GPIO ESP32", "Resistor & Transistor", "Header ke Modul Relay", "Terminal Daya Output"]
    data_act = [
        ["Kipas Pendingin Kotak (Fan)", "GPIO 39", "R1 (2kΩ) & Q1 (NPN)", "J_RELAY_FAN1 (3 Pin)", "J_OUT_FAN1 & J_HUBUNG_FAN1 (12V)"],
        ["Lampu UV Perangkap Hama", "GPIO 15", "R2 (2kΩ) & Q3 (NPN)", "J_RELAY_LAMP1 (3 Pin)", "J_OUT_LAMP1 & J_HUBUNG_LAMP1 (12V)"],
        ["Solenoid Valve (Penyiraman)", "GPIO 5", "R3 (2kΩ) & Q2 (NPN)", "J_RELAY_AKTU1 (3 Pin)", "J_OUT_AKTU1 & J_HUBUNG_AKTU1 (12V)"]
    ]
    create_custom_table([1.4, 0.9, 1.3, 1.4, 1.5], hdr_act, data_act)

    add_h2("2.4 Skematik Elektronika Lengkap (KiCad)")
    p = doc.add_paragraph()
    p.add_run(
        "Berikut adalah gambar skema pengkabelan elektronik menyeluruh dari sistem PCB tingkat atas yang dirancang "
        "menggunakan KiCad EDA (pengabdian.kicad_sch):"
    )

    add_image_box(img_schematic, "Gambar 2.2: Skematik Sirkuit Elektronika Lengkap PCB Tingkat Atas (KiCad EDA)", width=Inches(6.4))

    doc.add_page_break()

    # =========================================================================
    # BAB III: PANDUAN SISTEM DAYA PLTS & PCB TINGKAT BAWAH
    # =========================================================================
    add_h1("BAB III. SISTEM CATU DAYA MANDIRI ENERGI (PLTS & PCB BAWAH)")

    p = doc.add_paragraph()
    p.add_run(
        "Sistem NURTURE beroperasi sepenuhnya menggunakan energi terbarukan tenaga surya. Stabilitas operasional sangat bergantung "
        "pada kualitas perakitan kelistrikan dari panel surya hingga masuk ke PCB tingkat bawah."
    )

    add_h2("3.1 Diagram Alur Energi Listrik Sistem")
    p = doc.add_paragraph()
    p.add_run(
        "1. Panel Surya (Photovoltaic): Mengonversi radiasi sinar matahari menjadi energi listrik DC.\n"
        "2. Solar Charge Controller (SCC): Mengatur pengisian daya baterai (mencegah overcharge saat siang dan overdischarge saat malam).\n"
        "3. Baterai VRLA 12V 12Ah: Menyimpan cadangan energi yang sanggup menghidupkan seluruh sistem selama lebih dari 3 hari berawan/hujan tanpa matahari.\n"
        "4. Sensor Arus Dual INA226 (I2C):\n"
        "   • Modul 1 (Alamat I2C 0x44): Mengukur arus dan daya pengisian dari SCC ke Baterai (Charging Current).\n"
        "   • Modul 2 (Alamat I2C 0x45): Mengukur arus dan konsumsi daya aktual yang ditarik oleh seluruh sistem (Load Current).\n"
        "5. PCB Tingkat Bawah (Regulator Board): Mengambil input 12V dari baterai dan memecahnya menjadi rel 12V, 5V, dan 3.3V.\n"
        "6. PCB Tingkat Atas: Menerima tegangan bersih untuk ESP32-S3 dan sensor."
    )

    add_h2("3.2 Konfigurasi Modul Step-Down di PCB Tingkat Bawah")
    p = doc.add_paragraph()
    p.add_run(
        "Di PCB tingkat bawah dipasang modul regulator efisiensi tinggi (Buck Converter DC-DC, misalnya tipe LM2596 atau XL4015):\n"
        "• Modul Step-Down 5V: Disetel tepat pada tegangan keluaran 5.05V - 5.10V DC. Modul ini wajib mampu mengeluarkan arus konstan minimal 2.5 hingga 3.0 Ampere. Arus ini diperlukan saat ESP32-S3 memancarkan sinyal radio Wi-Fi sambil mengaktifkan relay.\n"
        "• Modul Step-Down 3.3V: Disetel tepat pada tegangan keluaran 3.30V DC untuk keperluan suplai logika sensor bertegangan rendah."
    )

    add_callout(
        "Pedoman Pemasangan Kabel Power (Mencegah Kebakaran & Drop Tegangan)",
        "1. Gunakan kabel serabut tembaga tebal (minimal ukuran AWG 18-20 atau penampang 0.75 mm²) untuk menghubungkan aki, SCC, dan input PCB bawah. Jangan pernah menggunakan kabel jumper tipis untuk jalur daya utama!\n"
        "2. Pasang Fuse / Sekring Pengaman DC (tipe tabung kaca atau sekring tancap otomotif ukuran 3A - 5A) pada kutub positif kabel aki menuju sistem sebagai proteksi korsleting.\n"
        "3. Pastikan baut terminal aki dan terminal blok PCB terkunci rapat. Baut kendor dapat menimbulkan percikan api dan membuat tegangan drop.",
        box_type="warning"
    )

    doc.add_page_break()

    # =========================================================================
    # BAB IV: LOGIKA OPERASIONAL & ATURAN OTOMASI SISTEM
    # =========================================================================
    add_h1("BAB IV. LOGIKA OPERASIONAL & ATURAN OTOMASI SISTEM")

    p = doc.add_paragraph()
    p.add_run(
        "Seluruh keputusan otomatisasi dikerjakan secara mandiri di dalam chip ESP32-S3 setiap detik tanpa memerlukan koneksi internet "
        "(sistem tetap menyiram dan menyalakan lampu meskipun jaringan Wi-Fi sedang terputus)."
    )

    add_h2("4.1 Otomasi Penyiraman Lahan (Smart Solenoid Valve)")
    p = doc.add_paragraph()
    p.add_run(
        "Penyiraman tanaman dilakukan secara terjadwal berdasarkan jam presisi dari modul RTC DS3231:\n"
        "• Jadwal Pagi: Solenoid valve terbuka otomatis tepat pada pukul 07:00:00 WIB dan menyiram selama 10 menit hingga pukul 07:10:00 WIB.\n"
        "• Jadwal Sore: Solenoid valve terbuka otomatis tepat pada pukul 17:20:00 WIB dan menyiram selama 10 menit hingga pukul 17:30:00 WIB.\n"
        "• Proteksi Batas Maksimum (Safety Timeout 15 Menit): Sistem dilengkapi fail-safe otomatis. Jika karena suatu gangguan program atau sensor valve menyala lebih dari 15 menit, mikrokontroler akan memutus daya valve secara paksa untuk mencegah kebanjiran lahan atau terbakarnya kumparan solenoid valve."
    )

    add_h2("4.2 Otomasi Kipas Pendingin Boks Panel (Smart Cooling Fan)")
    p = doc.add_paragraph()
    p.add_run(
        "Boks panel yang diletakkan di tengah sawah terbuka sangat rentan mengalami panas berlebih akibat terik matahari. "
        "Sistem melindungi komponen elektronik dengan aturan berikut:\n"
        "• Ambang Batas Suhu: Sensor DHT22 dalam boks memantau suhu secara kontinu. Jika suhu dalam panel mencapai atau melebihi 34.0 °C, kipas exhaust langsung dinyalakan.\n"
        "• Durasi Siklus: Kipas akan berputar selama 5 menit (300 detik) untuk menguras udara panas keluar boks, lalu mati otomatis.\n"
        "• Siklus Ulang: Setelah mati, sistem mengecek kembali suhu boks. Jika suhu masih di atas 34 °C, kipas akan melanjutkan pendinginan pada siklus berikutnya hingga suhu boks benar-benar aman."
    )

    add_h2("4.3 Otomasi Lampu Perangkap Hama (UV Light Trap)")
    p = doc.add_paragraph()
    p.add_run(
        "Lampu ultraviolet penarik serangga hama sawah dikendalikan otomatis berbasis jam malam (RTC DS3231):\n"
        "• Menyala otomatis pada pukul 18:00 WIB (petang/saat matahari terbenam).\n"
        "• Mati otomatis pada pukul 06:00 WIB (pagi hari).\n"
        "• Operasi ini ramah lingkungan karena menekan populasi hama nocturnal tanpa pestisida kimia berlebih."
    )

    add_h2("4.4 Pengiriman Data Nirkabel (Telemetri UDP & Cloud Dashboard)")
    p = doc.add_paragraph()
    p.add_run(
        "Sistem memisahkan jalur pemantauan menjadi dua kanal:\n"
        "1. Telemetri Real-Time Lokal (UDP Broadcast Port 4210): ESP32 memancarkan data mentah seluruh sensor setiap 1 detik sekali ke jaringan Wi-Fi lokal. Sangat berguna untuk teknisi memeriksa kondisi alat secara instan menggunakan laptop di lapangan.\n"
        "2. Cloud Dashboard ThingSpeak & Web: Data diunggah ke server cloud setiap 16 detik sekali agar grafik kondisi nutrisi tanah, suhu sawah, dan cadangan baterai PLTS dapat dipantau oleh petani dari mana saja lewat smartphone."
    )

    doc.add_page_break()

    # =========================================================================
    # BAB V: PANDUAN TROUBLESHOOTING & PEMELIHARAAN BAGI PETANI
    # =========================================================================
    add_h1("BAB V. PANDUAN TROUBLESHOOTING & PERAWATAN MANDIRI")

    p = doc.add_paragraph()
    p.add_run(
        "Petani atau pengurus kelompok tani (Poktan) dapat melakukan pemeriksaan awal dan penanganan mandiri "
        "jika terjadi kejanggalan pada sistem menggunakan panduan tabel berikut:"
    )

    add_h2("5.1 Tabel Diagnosa Masalah Lapangan (Troubleshooting)")

    hdr_trb = ["Gejala Masalah", "Kemungkinan Penyebab", "Cara Pemeriksaan", "Tindakan Perbaikan"]
    data_trb = [
        [
            "Lampu LED ESP32 redup, berkedip ngaco, atau data ThingSpeak macet.",
            "Tegangan 5V drop akibat tarikan arus aktuator (Brownout).",
            "Ukur tegangan pada pin 5V ESP32 menggunakan multimeter saat relay aktif.",
            "Putar potensio modul step-down 5V di PCB bawah agar tepat 5.1V. Pastikan kabel aki kencang."
        ],
        [
            "Data NPK di layar bernilai 0 atau 'ERROR MODBUS'.",
            "Kabel data RS485 A dan B terbalik, longgar, atau kabel sensor putus.",
            "Periksa soket J_NPK1. Cek apakah kabel kuning (A) dan biru (B) terpasang benar.",
            "Tukar posisi kabel A dan B pada soket J_NPK1. Periksa sambungan kabel sensor di dalam tanah."
        ],
        [
            "Air tidak mengalir keluar saat jam penyiraman tiba.",
            "Tandon air kosong, filter pipa tersumbat lumut, atau solenoid valve macet.",
            "Dengarkan apakah ada suara 'klik' dari relay pada pukul 07:00 atau 17:20.",
            "Jika relay berbunyi klik namun air tidak keluar, periksa tandon air dan bersihkan klep valve."
        ],
        [
            "Jadwal penyiraman atau lampu menyala di waktu yang salah.",
            "Waktu pada modul RTC DS3231 reset atau baterai koin CR2032 habis.",
            "Lihat tampilan jam di layar web dashboard apakah sesuai dengan jam dinding saat ini.",
            "Ganti baterai koin CR2032 pada modul RTC, lalu lakukan sinkronisasi waktu jam via kode."
        ],
        [
            "Kipas boks tidak mau menyala padahal boks terasa panas menyengat.",
            "Sensor DHT22 kotak panel kendor atau suhu belum menyentuh batas 34 °C.",
            "Cek pembacaan 'Suhu Dalam' di dashboard web apakah sudah melewati angka 34.0 °C.",
            "Kencangkan soket J_DHT_BOX1. Cek kabel kipas 12V di terminal J_OUT_FAN1."
        ],
        [
            "Grafik arus baterai bernilai negatif (-) di siang hari.",
            "Arah pembacaan modul INA226 terbalik atau baterai sedang menyuplai beban.",
            "Lihat apakah panel surya tertutup dedaunan/kotoran burung sehingga pengisian lemah.",
            "Bersihkan permukaan panel surya dari debu dan kotoran agar arus charging kembali positif."
        ]
    ]
    create_custom_table([1.5, 1.4, 1.6, 2.0], hdr_trb, data_trb)

    add_h2("5.2 Jadwal Pemeliharaan Rutin Perangkat")
    p = doc.add_paragraph()
    p.add_run(
        "Untuk menjaga umur pakai sistem hingga bertahun-tahun, kelompok tani disarankan menjalankan perawatan berkala:\n\n"
        "1. Pemeliharaan Mingguan:\n"
        "   • Bersihkan kaca Panel Surya menggunakan kain basah dari debu tanah sawah dan kotoran burung.\n"
        "   • Periksa kebersihan saringan/filter air sebelum masuk ke Solenoid Valve agar pasir tidak mengganjal klep.\n"
        "   • Bersihkan wadah penampung serangga pada Lampu UV Trap.\n\n"
        "2. Pemeliharaan Bulanan:\n"
        "   • Buka kotak panel kendali, periksa apakah ada serangga/semut yang bersarang di sela-sela PCB.\n"
        "   • Periksa kekencangan baut terminal aki dan konektor kabel daya.\n"
        "   • Periksa lubang ventilasi dan bersihkan debu yang menempel pada bilah kipas pendingin.\n\n"
        "3. Pemeliharaan Setiap Musim Tanam:\n"
        "   • Cabut dan bersihkan probe stainless steel sensor NPK 7-in-1 dari kerak tanah kering atau sisa pupuk.\n"
        "   • Periksa tegangan Baterai VRLA saat sore hari tanpa beban (tegangan sehat berada di atas 12.6V DC).\n"
        "   • Periksa kondisi baterai koin CR2032 pada modul RTC DS3231."
    )

    add_callout(
        "Layanan Bantuan Teknis",
        "Jika ditemukan kerusakan fisik pada papan sirkuit PCB atau kegagalan transmisi data yang tidak dapat diselesaikan melalui buku panduan ini, silakan hubungi Tim Pengabdian Masyarakat UGM atau pengelola teknis Poktan Margorejo.",
        box_type="info"
    )

    # ---------------- SAVE DOCUMENT ----------------
    out_path = r"c:\PKM Margorejo\Buku_Panduan_Wiring_NURTURE_Margorejo.docx"
    doc.save(out_path)
    print(f"Document created successfully at: {out_path}")

if __name__ == "__main__":
    create_manual()
