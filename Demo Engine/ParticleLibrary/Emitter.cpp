#include "Emitter.h"
#include "ParticleManager.h"
#include "Timer.h"
#include <glad/glad.h>
#include "PlaneImporter.h"
#include <Brofiler/Brofiler.h>


Emitter::Emitter(ParticleManager* parent, float* emitterPos, int maxParticles): parent(parent), globalObjPos(emitterPos[0])
{
	ParticleColor startColor;
	colorList.push_back(startColor);
	plane = new PlaneImporter(maxParticles);

	ChangeMaxParticles(maxParticles);
}

void Emitter::ChangeMaxParticles(int maxParticles)
{
	if (maxParticles <= 0)
		maxParticles = 1;

	particleActiveBool.resize(maxParticles);
	particleLife.resize(maxParticles);
	particlePosition.resize(maxParticles);
	particleDirection.resize(maxParticles);
	particleGravity.resize(maxParticles);
	particleSpeed.resize(maxParticles);
	particleAcceleration.resize(maxParticles);
	particleAngleRot.resize(maxParticles);
	particleAngleVel.resize(maxParticles);
	particleAngleAccel.resize(maxParticles);
	particleSize.resize(maxParticles);
	particleSizeTime.resize(maxParticles);
	
	plane->SetDynamicValues(maxParticles);
}

Emitter::~Emitter()
{
	StopEmitter();
	delete plane;
}

void Emitter::Update(float dt)
{
	if (runningTime = TimerState::StatePlayed)
	{
		if (particlesEmition > 0)
		{
			float time = emitterTimer.GetTime() / 1000.0f;
			if (time > secParticleCreation)
			{
				int particlesToCreate = (time / (1.0f / particlesEmition));
				CreateParticles(particlesToCreate, globalObjPos, shapeEmitter);

				secParticleCreation = (1.0f / particlesEmition);

				emitterTimer.Play();
			}
		}
		if (isBurst)
		{
			float time = burstTimer.GetTime() / 1000.0f;
			if (time > burstSeconds && !onceBurst)
			{
				CreateParticles(parent->GetRandomNum(minBurst, maxBurst), globalObjPos, burstShapeEmitter);

				if (burstSeconds == 0)
				{
					onceBurst = true;
					burstTimer.Stop();
				}
				else
					burstTimer.Play();
			}
		}

		float initialActive = particleActive;
		for (int i = 0; i < initialActive; ++i)
		{
			if (particleActiveBool[i])
			{
				if (particleLife[i].x < particleLife[i].y)
					particleLife[i].x += dt;
				else
				{
					//Deactivate the particle
					particleActive--;

					particleLife[i] = particleLife[particleActive];
					particlePosition[i] = particlePosition[particleActive];
					particleDirection[i] = particleDirection[particleActive];
					particleGravity[i] = particleGravity[particleActive];
					particleSpeed[i] = particleSpeed[particleActive];
					particleAcceleration[i] = particleAcceleration[particleActive];
					particleAngleRot[i] = particleAngleRot[particleActive];
					particleAngleVel[i] = particleAngleVel[particleActive];
					particleAngleAccel[i] = particleAngleAccel[particleActive];
					particleSize[i] = particleSize[particleActive];
					particleSizeTime[i] = particleSizeTime[particleActive];

					particleActiveBool[particleActive] = false;

					changeValues = true;
				}
			}
		}

		if (changeValues)
			SetNewBuffers();
	}
}

void Emitter::SetNewBuffers()
{
	//Life
	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(PartVec2), &particleLife[0]);

	//Position
	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[2]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(PartVec3), &particlePosition[0]);

	//Direction
	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[3]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(PartVec3), &particleDirection[0]);

	//Speed
	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[4]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(float), &particleSpeed[0]);

	//Acceleration
	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[5]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(float), &particleAcceleration[0]);

	//Gravity - Acceleration 3D
	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[6]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(PartVec3), &particleGravity[0]);

	//Rotation
	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[7]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(float), &particleAngleRot[0]);

	//Rotation Velocity
	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[8]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(float), &particleAngleVel[0]);

	//Rotation Acceleration
	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[9]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(float), &particleAngleAccel[0]);

	//Size
	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[10]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(float), &particleSize[0]);

	//Size Over Time
	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[11]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(float), &particleSizeTime[0]);
}

void Emitter::CreateParticles(int numParticles, PartVec3 globalPosition, ShapeEmitter emitter)
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::PapayaWhip);

	for (int i = 0; i < numParticles; ++i)
	{
		//int particleId = 0;
		if (particleActive < particleLife.size())
		{
			globalPosition += GetRandomPos(emitter);
			//Create the particle in the correctly slot in the pool
			SetParticleValues(globalPosition);
			particleActive++;
		}
		else
			break;
	}
	changeValues = true;
}

void Emitter::SetParticleValues(PartVec3 pos)
{
	//Save all the initial values 
	particleLife[particleActive] = PartVec2(0.0f, parent->GetRandomNum(startValues.life.x, startValues.life.y));
	particlePosition[particleActive] = pos;
	particleDirection[particleActive] = startValues.particleDirection;
	particleGravity[particleActive] = startValues.gravity;
	particleSpeed[particleActive] = parent->GetRandomNum(startValues.speed.x, startValues.speed.y);
	particleAcceleration[particleActive] = parent->GetRandomNum(startValues.acceleration.x, startValues.acceleration.y);
	particleAngleRot[particleActive] = parent->GetRandomNum(startValues.rotation.x, startValues.rotation.y);
	particleAngleVel[particleActive] = parent->GetRandomNum(startValues.angularVelocity.x, startValues.angularVelocity.y);
	particleAngleAccel[particleActive] = parent->GetRandomNum(startValues.angularAcceleration.x, startValues.angularAcceleration.y);
	particleSize[particleActive] = parent->GetRandomNum(startValues.size.x, startValues.size.y);
	particleSizeTime[particleActive] = parent->GetRandomNum(startValues.sizeOverTime.x, startValues.sizeOverTime.y);

	particleActiveBool[particleActive] = true;
}
void Emitter::SetShapeEmitter(ShapeEmitter shape)
{
	shapeEmitter = shape;
}

ShapeEmitter Emitter::GetShapeEmitter() const
{
	return shapeEmitter;
}

void Emitter::SetBurstShapeEmitter(ShapeEmitter shape)
{
	burstShapeEmitter = shape;
}

ShapeEmitter Emitter::GetBurstShapeEmitter() const
{
	return burstShapeEmitter;
}

void Emitter::SetGlobalPos(float* globalPos)
{
	globalObjPos.x = globalPos[0];
	globalObjPos.y = globalPos[1];
	globalObjPos.z = globalPos[2];
}


void Emitter::Draw(unsigned int shaderUuid)
{

	glUniform1i(glGetUniformLocation(shaderUuid, "colorSize"), isMulticolor ? colorList.size() : 1);
	glUniform1i(glGetUniformLocation(shaderUuid, "useTexture"), useTexture);

	glUniform1i(glGetUniformLocation(shaderUuid, "isAnimated"), isParticleAnimated);
	glUniform1i(glGetUniformLocation(shaderUuid, "textRows"), textureRows);
	glUniform1i(glGetUniformLocation(shaderUuid, "textColumns"), textureColumns);
	glUniform1f(glGetUniformLocation(shaderUuid, "animatedTime"), animationSpeed);

	int cont = 0;
	for (std::list<ParticleColor>::iterator iter = colorList.begin(); iter != colorList.end(); ++iter, ++cont)
	{
		std::string name = "colors[" + std::to_string(cont) + "]";
		glUniform4fv(glGetUniformLocation(shaderUuid, name.c_str()), 4, &(*iter).color.x);

		name = "positions[" + std::to_string(cont) + "]";
		glUniform1fv(glGetUniformLocation(shaderUuid, name.c_str()), 1, &(*iter).position);

		if (!isMulticolor)
			break;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	//Bind VAO
	glBindVertexArray(plane->VAO);

	// Set mesh attributes
	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[0]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	// textCoords
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));

	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[1]);
	//Life
	glVertexAttribDivisor(2, 1);
	glEnableVertexAttribArray(2);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particleActive * sizeof(PartVec2), &particleLife[0]);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[2]);
	//Position
	glVertexAttribDivisor(3, 1);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[3]);
	//Direction
	glVertexAttribDivisor(4, 1);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[4]);
	//Speed
	glVertexAttribDivisor(5, 1);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[5]);
	//Acceleration
	glVertexAttribDivisor(6, 1);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[6]);
	//Gravity - Acceleration 3D
	glVertexAttribDivisor(7, 1);
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[7]);
	//Rotation
	glVertexAttribDivisor(8, 1);
	glEnableVertexAttribArray(8);
	glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[8]);
	//Rotation Velocity
	glVertexAttribDivisor(9, 1);
	glEnableVertexAttribArray(9);
	glVertexAttribPointer(9, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[9]);
	//Rotation Acceleration
	glVertexAttribDivisor(10, 1);
	glEnableVertexAttribArray(10);
	glVertexAttribPointer(10, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[10]);
	//Size
	glVertexAttribDivisor(11, 1);
	glEnableVertexAttribArray(11);
	glVertexAttribPointer(11, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, plane->VBO[11]);
	//Size Over Time
	glVertexAttribDivisor(12, 1);
	glEnableVertexAttribArray(12);
	glVertexAttribPointer(12, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, particleActive);

	glBindVertexArray(0);
}

bool Emitter::SaveCameraDistance()
{
	bool ret = false;
	if (parent->cameraPos)
	{
		float x = (*parent->cameraPos).x - globalObjPos.x;
		float y = (*parent->cameraPos).y - globalObjPos.y;
		float z = (*parent->cameraPos).z - globalObjPos.z;

		cameraDist = x * x + y * y + z * z;
		ret = true;
	}
	return ret;
}

void Emitter::StartEmitter()
{
	runningTime = TimerState::StatePlayed;
	onceBurst = false;
	emitterTimer.Play();
	burstTimer.Play();
}

void Emitter::StopEmitter()
{
	for (int i = 0; i < particleActive; ++i)
	{
		if (particleActiveBool[i])
		{
			particleActiveBool[i] = false;
		}
	}

	runningTime = TimerState::StateStopped;
	secParticleCreation = 0.0f;

	particleActive = 0;
	emitterTimer.Stop();
	burstTimer.Stop();
}

void Emitter::PauseEmitter()
{
	runningTime = TimerState::StatePaused;
}

PartVec3 Emitter::GetRandomPos(ShapeEmitter emitter)
{
	PartVec3 randomPos = PartVec3(0.0f);

	switch (emitter)
	{
	case BoxShape:
		//Box Size
		randomPos.x = parent->GetRandomNum(-boxShapeSize.x / 2, boxShapeSize.x / 2);
		randomPos.y = parent->GetRandomNum(-boxShapeSize.y / 2, boxShapeSize.y / 2);
		randomPos.z = parent->GetRandomNum(-boxShapeSize.z / 2, boxShapeSize.z / 2);

		startValues.particleDirection = PartVec3(0.0f, 1.0f, 0.0f);
		break;
	case SphereShape:
	case SphereShapeCenter:
	case SphereShapeBorder:
		//Sphere rad
		randomPos.x = parent->GetRandomNum(-1.0f, 1.0f);
		randomPos.y = parent->GetRandomNum(-1.0f, 1.0f);
		randomPos.z = parent->GetRandomNum(-1.0f, 1.0f);

		randomPos = randomPos.Normalize();
		startValues.particleDirection = randomPos;

		if (shapeEmitter == SphereShape)
			randomPos *= parent->GetRandomNum(0.0f, sphereShapeRad);
		else if (shapeEmitter == SphereShapeCenter)
			randomPos = PartVec3(0.0f);
		else if (shapeEmitter == SphereShapeBorder)
			randomPos = startValues.particleDirection * sphereShapeRad;
		break;
	case ConeShape:
		//The position is always 0. We only change the direction
	{
		PartVec3 destination;
		destination.x = parent->GetRandomNum(-coneShapeRad, coneShapeRad);
		destination.y = coneShapeHeight;
		destination.z = parent->GetRandomNum(-coneShapeRad, coneShapeRad);

		startValues.particleDirection = destination.Normalize();

	}
	break;
	default:
		break;
	}


	return randomPos;
}

