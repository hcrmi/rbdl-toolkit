#include "ArrowFieldModelExtention.h"
#include "toolkit_errors.h"

#include <iostream>

#include <Qt3DCore>
#include <Qt3DRender>
#include <Qt3DExtras>

using namespace Qt3DCore;
using namespace Qt3DRender;
using namespace Qt3DExtras;

ArrowFieldModelExtention::ArrowFieldModelExtention(
	Qt3DRender::QMesh *arrow_mesh, 
	QString group_name,
	QColor arrow_color) : 
		arrow_mesh(arrow_mesh),
		max_time(0.),
		arrow_count(0),
		arrow_color(arrow_color),
		group_name(group_name),
		WrapperExtention()
{
	arrow_field_root = new QEntity;
	arrow_field_root->setProperty("Scene.ObjGroup", QVariant(group_name));
}

std::string ArrowFieldModelExtention::getExtentionName() {
	return group_name.toStdString();
}

void ArrowFieldModelExtention::update(float current_time) {
	if (arrow_positions.size() == 0 || arrow_count == 0)
		return;

	unsigned int time_index = 0;
	for (; time_index < arrow_times.size(); time_index++) {
		if (time_index == arrow_times.size()-1) break;
		if (arrow_times[time_index] <= current_time && arrow_times[time_index+1] > current_time) {
			break;
		}
	}

	Matrix3fd pos_frame;
	Matrix3fd dir_frame;
	if (time_index == arrow_times.size()-1) {
		pos_frame = arrow_positions[arrow_positions.size()-1]; 
		dir_frame = arrow_directions[arrow_directions.size()-1]; 
	} else {
		float start_time = arrow_times[time_index];
		float end_time = arrow_times[time_index+1];
		float time_fraction = (current_time - start_time) / (end_time - start_time);

		Matrix3fd pos = arrow_positions[time_index] + time_fraction * 
			(arrow_positions[time_index+1] - arrow_positions[time_index]);

		Matrix3fd dir = arrow_directions[time_index] + time_fraction * 
			(arrow_directions[time_index+1] - arrow_directions[time_index]);

		pos_frame = pos;
		dir_frame = dir;
	}
	for (int i=0; i<arrow_count;i++) {
		QVector3D trans;
		trans[0] = pos_frame(0,i);
		trans[1] = pos_frame(1,i);
		trans[2] = pos_frame(2,i);
		arrow_transforms[i]->setTranslation(trans);
		arrow_transforms[i]->setRotation(QQuaternion(0, 0, 0, 0));
	}
}

QEntity* ArrowFieldModelExtention::getVisual() {
	if (arrow_positions.size() == 0 || arrow_count == 0)
		return nullptr;

	for (int i=0;i<arrow_count;i++) {
		QEntity* arrow_entity = new QEntity(arrow_field_root);

		Qt3DExtras::QDiffuseSpecularMaterial* material = new QDiffuseSpecularMaterial;
		material->setAmbient(arrow_color);
		material->setAlphaBlendingEnabled(true);

		Qt3DCore::QTransform* transform = new Qt3DCore::QTransform;
		arrow_transforms.push_back(transform);

		arrow_entity->addComponent(arrow_mesh);
		arrow_entity->addComponent(material);
		arrow_entity->addComponent(transform);
	}

	update(0.);

	return arrow_field_root;
}


void ArrowFieldModelExtention::addArrowFieldFrame(float time, const Matrix3fd& pos, const Matrix3fd& dir) {
	if (arrow_count == 0) {
		arrow_count = pos.size() / 3;
	}

	if (pos.size() != dir.size()) {
		throw RBDLToolkitError("Mismatch in amount of positions and directions provided!");
	}
	if (pos.size()/3 != arrow_count) {
		throw RBDLToolkitError("Amount of arrows does not match the amount previously provided!");
	}

	arrow_times.push_back(time);
	arrow_positions.push_back(pos);
	arrow_directions.push_back(dir);
}
