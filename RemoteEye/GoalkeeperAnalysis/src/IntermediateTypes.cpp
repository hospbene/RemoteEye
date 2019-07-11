#include "IntermediateTypes.h"
#include <utility>

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <QDebug>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

ScreenspaceGazeResult::ScreenspaceGazeResult():
valid(false),
position(-1,-1)
{
	
}

ScreenspaceGazeResult::ScreenspaceGazeResult(cv::Point2f position):
valid(true),
position(position)
{
	
}

SingleEyeFeatures::SingleEyeFeatures():
valid(false),
glints(num_glints, cv::Point2f(-1,-1)),
pupil_center_(-1, -1)
{
	
}

SingleEyeFeatures::SingleEyeFeatures(cv::Point2f pupil_center, std::vector<cv::Point2f> glints):
valid(true),
glints(std::move(glints)),
pupil_center_(pupil_center)
{
	
}

SingleEyeFeatures::operator gazeestimation::PupilCenterGlintInput() const
{
	gazeestimation::PupilCenterGlintInput result;
	result.pupil_center = gazeestimation::make_vec2(pupil_center().x, pupil_center().y);

	for(const auto& glint : glints)
	{
		result.glints.push_back(gazeestimation::make_vec2(glint.x, glint.y));
	}

	return result;
}

SingleEyeFeatures::operator bool() const { return valid; }

cv::Point2f SingleEyeFeatures::glint(int index) const
{
	GA_ASSERT(index >= 0 || index < glints.size());
	return glints[index];
}

bool SingleEyeFeatures::glintValid(int index) const
{
	GA_ASSERT(index >= 0 || index < glints.size());
	return glints[index].x >= 5 && glints[index].y >= 5;
}

bool SingleEyeFeatures::pupilValid() const
{
	return pupil_center_.x >= 5 && pupil_center_.y >= 5;
}

bool SingleEyeFeatures::allValid() const
{
	if (!pupilValid())
		return false;

	int valid_glints = 0;
	for(size_t i = 0; i < glints.size(); i++)
	{
		if (glintValid(i))
			valid_glints++;
	}

	if (valid_glints < 2)
		return false;

	return true;
}

DetectedFeatures::DetectedFeatures(const PupilGlintCombiInstance& source)
{
	if(source.detectedLeft){
		left_eye = SingleEyeFeatures(source.pupilLeft, { source.glintLeft_1, source.glintLeft_2, source.glintLeft_3 });
	}
	if(source.detectedRight)
	{
		right_eye = SingleEyeFeatures(source.pupilRight, { source.glintRight_1, source.glintRight_2, source.glintRight_3 });
	}
}

SingleEyeFeatures DetectedFeatures::eye(EyePosition position) const
{
	if (position == EyePosition::LEFT)
		return left_eye;
	else if (position == EyePosition::RIGHT)
		return right_eye;
	return SingleEyeFeatures();
}

void PupilGlintCombiInstance::log() const
{
	qDebug() << "PR " << pupilRight.x << " / " << pupilRight.y
		<< "  G1R " << glintRight_1.x << " / " << glintRight_1.y
		<< "  G2R " << glintRight_2.x << " / " << glintRight_2.y
		<< "  G3R " << glintRight_3.x << " / " << glintRight_3.y
		<< "  |||||| PL " << pupilLeft.x << "  / " << pupilLeft.y
		<< "  G1L " << glintLeft_1.x << " / " << glintLeft_1.y
		<< "  G2L " << glintLeft_2.x << " / " << glintLeft_2.y
		<< "  G3L " << glintLeft_3.x << " / " << glintLeft_3.y;
}

void PupilGlintCombiInstance::toFile(std::ostream* file) const
{
	*file << pupilRight.x << "," << pupilRight.y << ","
		<< glintRight_1.x << "," << glintRight_1.y << ","
		<< glintRight_2.x << "," << glintRight_2.y << ","
		<< glintRight_3.x << "," << glintRight_3.y << ","
		<< pupilLeft.x << "," << pupilLeft.y << ","
		<< glintLeft_1.x << "," << glintLeft_1.y << ","
		<< glintLeft_2.x << "," << glintLeft_2.y << ","
		<< glintLeft_3.x << "," << glintLeft_3.y << "," << std::endl;
}
