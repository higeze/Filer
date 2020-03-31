#include "Band.h"

void CBand::SetStart(const FLOAT start, bool notify)
{
	if (m_start != start) {
		m_start = start;
		if (notify) {
			OnPropertyChanged(L"start");
		}
	}
}

void CBand::SetLength(const FLOAT length, bool notify)
{
	if (m_length != length) {
		m_length = std::clamp(
			length,
			m_isMinLengthFit ? GetFitLength() : m_minLength,
			m_isMaxLengthFit ? GetFitLength() : m_maxLength);

		if (notify) {
			OnPropertyChanged(L"size");
		}
	}
}

void CBand::FitLength(bool notify)
{
	SetLength(GetFitLength(), notify);
}


void CBand::BoundLength(bool notify)
{
	auto length = m_length;
	SetLength(length, notify);
}