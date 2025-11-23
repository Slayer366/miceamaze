// Copyright (c) 2012, River CHAMPEIMONT
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#include "RenderFlatText.h"
#include "Program.h"

static constexpr float FONT_NDC_SCALE = 10.0f;

TTF_Font *RenderFlatText::defaultFont = nullptr;
GLuint RenderFlatText::textTexture = 0;
std::string RenderFlatText::lastText = "";
int RenderFlatText::lastW = 0;
int RenderFlatText::lastH = 0;

// Helper: compute a "sensible" point size from the current window/viewport height.
// Adjust multiplier to taste. Using viewport height ensures text scales with resolution.
static int compute_point_size_from_window() {
    // Get current window pixel height.
    SDL_Window *w = Program::getInstance()->window;
    int h = 480;
    if (w) {
        SDL_GetWindowSize(w, NULL, &h); // Window drawable size in pixels
    } else {
        h = Program::getInstance()->screenHeight > 0 ? Program::getInstance()->screenHeight : h;
    }
    // pointSize fraction of window height. Tweak 0.055 if you want default glyphs bigger/smaller.
    int pts = (int)roundf((float)h * 0.055f);
    if (pts < 8) pts = 8;
    return pts;
}

void RenderFlatText::init() {
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (TTF_WasInit() == 0) {
        if (TTF_Init() < 0) {
            Functions::fatalError(std::string("TTF_Init failed: ") + TTF_GetError());
            SDL_Quit();
        }
    }

	string fontPath = Program::getInstance()->dataPath + "/fonts/DejaVuSans.ttf";

    // compute dynamic point size from current window/viewport
    int pointSize = compute_point_size_from_window();

    if (!defaultFont) {
        defaultFont = TTF_OpenFont(fontPath.c_str(), pointSize);
    } else if (!defaultFont) {
        Functions::fatalError(std::string("Failed to load font: ") + TTF_GetError());
    }
}

void RenderFlatText::reinit() {
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (defaultFont) {
        TTF_CloseFont(defaultFont);
        defaultFont = nullptr;
    }

    if (textTexture) {
        glDeleteTextures(1, &textTexture);
        textTexture = 0;
        lastText.clear();
        lastW = lastH = 0;
    }

    init();
}

// align: -1 = left (original position = left of the text)
// align:  0 = center (original position = middle of the text)
// align:  1 = right (original position = right of the text)
void RenderFlatText::render(string s, int align) {

    if (s.length() == 0) {
        return;
    }

    // Measure the text in pixels (similar to GLC string metric)
    GLint px_w = 0, px_h = 0;
    if (TTF_SizeUTF8(defaultFont, s.c_str(), &px_w, &px_h) != 0) {
        return;
    }

    int outW, outH;

    std::string cacheKey;
    {
        // pointer as hex + text
        std::ostringstream oss;
        oss << reinterpret_cast<void*>(defaultFont) << "|" << s;
        cacheKey = oss.str();
    }
    if (textTexture != 0 && cacheKey == RenderFlatText::lastText) {
        outW = RenderFlatText::lastW;
        outH = RenderFlatText::lastH;
        return; // Also stops leak
    }

    SDL_Color white = {255,255,255,255};
    SDL_Surface *surf = TTF_RenderUTF8_Blended(defaultFont, s.c_str(), white);
    if (!surf) {
        return;
    }

    SDL_Surface *surf_conv = SDL_ConvertSurfaceFormat(surf, SDL_PIXELFORMAT_RGBA32, 0);
    SDL_FreeSurface(surf);
    if (!surf_conv) {
        return;
    }

    // delete previous texture if exists
//    if (textTexture != 0) {
//        glDeleteTextures(1, &textTexture);
//        textTexture = 0;
//    }

//    glGenTextures(1, &textTexture);
    glBindTexture(GL_TEXTURE_2D, textTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // ensure tight packing for arbitrary width
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf_conv->w, surf_conv->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf_conv->pixels);

    // restore previous unpack alignment if necessary
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    outW = surf_conv->w;
    outH = surf_conv->h;

    // cache key: store the font pointer + string so different fonts don't reuse same texture
    RenderFlatText::lastText = cacheKey;
    RenderFlatText::lastW = outW;
    RenderFlatText::lastH = outH;

    SDL_FreeSurface(surf_conv);

    // Query viewport to map pixel-size texture -> normalized coordinates used by UI.
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    const float vpW = (float)vp[2];
    const float vpH = (float)vp[3];

    // Convert pixel dimensions to normalized (-1..1) units: text_ndc = (pixels / vp) * 2
    // Caller is using coordinates where full width = 2 units (-1..1), so this matches.
    const float px_to_ndc = (2.0f / vpW) * FONT_NDC_SCALE; // multiply X by px_to_ndc to get NDC units
    const float py_to_ndc = (2.0f / vpH) * FONT_NDC_SCALE;

    const float w_ndc = ((float)px_w) * px_to_ndc;
    const float h_ndc = ((float)px_h) * py_to_ndc;

    // Save some GL state
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textTexture);

    // We do not alter the projection. DRAW IN CURRENT MODELVIEW SPACE.
    // Apply alignment in modelview (so it composes naturally with caller's translate/scale)
    glPushMatrix();

    if (align == 0 || align == 1) {
        if (align == 1) {
        // align:  1 = right (original position = right of the text)
        glTranslatef(-w_ndc, 0.0f, 0.0f);
        } else if (align == 0) {
        // align:  0 = center (original position = middle of the text)
        glTranslatef(-w_ndc * 0.5f, -0.25f, 0.0f);
        }
    } else if (align == -1) {
      // align: -1 = left (original position = left of the text)
      glTranslatef(0.0f, 0.0f, 0.0f);
    }

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f,     0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(w_ndc,    0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(w_ndc,    h_ndc);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f,     h_ndc);
    glEnd();

    glPopMatrix();
    glPopAttrib();
}
