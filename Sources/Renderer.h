// ----------------------------------------------
// Polytechnique - INF584 "Image Synthesis"
//
// Base code for practical assignments.
//
// Copyright (C) 2023 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

#pragma once

#include "Scene.h"

class Renderer {
public:
	virtual void render (const std::shared_ptr<Scene> scenePtr) = 0;
};