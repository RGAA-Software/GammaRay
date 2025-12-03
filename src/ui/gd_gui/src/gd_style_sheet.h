#pragma once

namespace GD {
const std::string KPlayerSliderCss = R"(
QSlider::groove:horizontal {
	background: #3D8BF9;
	height: 4px;
	border-radius: 2px;
}

QSlider::handle:horizontal {
	width: 12px;
	height: 12px;
	background: #ffffff;
	margin: -4px 0px; 
	border-radius: 6px;  
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
	width: 8px;
	height: 8px;
	background: #ffffff;
	margin: -2px 0px; 
	border-radius: 3px;  
}

QSlider::add-page:horizontal {
	background: #DEE0E5;
	border-radius: 2px;
}
)";
}