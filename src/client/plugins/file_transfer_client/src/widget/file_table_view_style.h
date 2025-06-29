#pragma once

namespace tc {
const std::string KPlayerSliderCss = R"(
QSlider::groove:horizontal {
	background: #3D8BF9;
	height: 6px;
	border-radius: 3px;
}

QSlider::handle:horizontal {
	width: 14px;
	height: 20px;
	background: #3D8BF9;
	margin: -6px 0px; 
	border: 2px solid #ffffff;
	border-radius: 9px;  
}

QSlider::add-page:horizontal {
	background: #DEE0E5;
	border-radius: 3px;
}
)";


const std::string KVoiceSliderCss = R"(
QSlider::groove:horizontal {
	background: #3D8BF9;
	height: 4px;
	border-radius: 2px;
}

QSlider::handle:horizontal {
	width: 7px;
	height: 10px;
	background: #3D8BF9;
	margin: -3px 0px; 
	border: 1px solid #ffffff;
	border-radius: 4px;  
}

QSlider::add-page:horizontal {
	background: #DEE0E5;
	border-radius: 2px;
}
)";


const std::string KEncSliderCss = R"(
QSlider::groove:horizontal {
	background: #3D8BF9;
	height: 4px;
	border-radius: 2px;
}
QSlider::add-page:horizontal {
	background: #DEE0E5;
	border-radius: 2px;
}
)";

const std::string  KFileTableStyle = R"(
QLineEdit { background-color: white; }
QTableView{
    outline:none; 
    border:0px;
    border:none;
    background-color: #ffffff;
    alternate-background-color: #f6f6f6;
}
QHeaderView::section {
    padding: 5px;
    margin:0px;
    color:#333333;
    border:1px solid #D0D0D0;
    background:#F5F5F5;
}
QTableView QTableCornerButton::section {
    background:#F5F9FE;
    }
QTableView::item{
    padding-left: 5px;
    margin: 0px;
    border:none;
    color: #333333;
}
QTableView::item:selected{
    border:none;
    /*background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 #636363, stop:1 #575757);*/
    color: #000000;
    background-color: #E0F0F6;
}
)";

const std::string kScrollBarStyle = R"(QScrollBar:vertical
{
    width:8px;
    background:rgba(0,0,0,0%);
    margin:0px,px,0px,0px;
    padding-top:4px;   
    padding-bottom:4px;
}
QScrollBar::handle:vertical
{
    width:8px;
    background:rgba(150,150,150,50%);
    border-radius:4px;   
    min-height:20;
}
QScrollBar::handle:vertical:hover
{
    width:8px;
    background:rgba(180,180,180,50%);
    border-radius:4px;
    min-height:20;
}
QScrollBar::add-line:vertical  
{
    height:9px;width:8px;
    border-image:url(:/images/a/3.png);
    subcontrol-position:bottom;
}
QScrollBar::sub-line:vertical 
{
    height:9px;width:8px;
    border-image:url(:/images/a/1.png);
    subcontrol-position:top;
}
QScrollBar::add-line:vertical:hover  
{
    height:9px;width:8px;
    border-image:url(:/images/a/4.png);
    subcontrol-position:bottom;
}
QScrollBar::sub-line:vertical:hover  
{
    height:9px;width:8px;
    border-image:url(:/images/a/2.png);
    subcontrol-position:top;
}
QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical   
{
    background:rgba(0,0,0,10%);
    border-radius:4px;
})";

}