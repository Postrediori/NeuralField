#include "stdafx.h"
#include "Matrix.h"
#include "GraphicsUtils.h"
#include "GraphicsLogger.h"
#include "ContourPlot.h"
#include "ContourFill.h"

bool ContourFill::Update(matrix_t* points, const hmm_vec4& area, double t) {
    threshold = t;

    this->area = area;

    int xdiv = points->cols - 1;
    int ydiv = points->rows - 1;
    
    double dX = (area.Y - area.X) / static_cast<double>(xdiv);
    double dY = (area.W - area.Z) / static_cast<double>(ydiv);

    int max_idx = xdiv * ydiv;

    triangles_t all_triangles;
    all_triangles.reserve(max_idx * 3 * 4);

#ifdef USE_OPENMP
#pragma omp parallel
#endif
    {
        triangles_t triangles;

#ifdef USE_OPENMP
#pragma omp for nowait
#endif
        for (int idx=0; idx<max_idx; idx++) {
            int i = idx % xdiv;
            int j = idx / xdiv;

            float y = area.Z + static_cast<float>(j) * dY;
            float x = area.X + static_cast<float>(i) * dX;
        
            vals_t vals;
            vals.v[0] = points->data[(j  )*(xdiv+1)+(i  )] - threshold;
            vals.v[1] = points->data[(j  )*(xdiv+1)+(i+1)] - threshold;
            vals.v[2] = points->data[(j+1)*(xdiv+1)+(i+1)] - threshold;
            vals.v[3] = points->data[(j+1)*(xdiv+1)+(i  )] - threshold;

            SquareFlags flags = CellType(vals);

            if (flags == SquareFlags::SouthWest || flags == SquareFlags::NorthWest ||
                flags == SquareFlags::NorthEast || flags == SquareFlags::SouthEast
                || flags == (SquareFlags::All ^ SquareFlags::SouthWest)
                || flags == (SquareFlags::All ^ SquareFlags::NorthWest)
                || flags == (SquareFlags::All ^ SquareFlags::NorthEast)
                || flags == (SquareFlags::All ^ SquareFlags::SouthEast)) {
                // One corner
                float x1 = 0.0f, y1 = 0.0f;
                float x2 = 0.0f, y2 = 0.0f;
                float sx = dX, sy = dY;

                if (flags == SquareFlags::SouthWest ||
                    flags == (SquareFlags::All ^ SquareFlags::SouthWest)) {
                    x1 = x + sx * ValuesRatio(vals, 0, 1);
                    y1 = y;
                    x2 = x;
                    y2 = y + sy * ValuesRatio(vals, 0, 3);
                }
                else if (flags == SquareFlags::NorthWest ||
                    flags == (SquareFlags::All ^ SquareFlags::NorthWest)) {
                    x1 = x + sx;
                    y1 = y + sy * ValuesRatio(vals, 1, 2);
                    x2 = x + sx * ValuesRatio(vals, 0, 1);
                    y2 = y;
                }
                else if (flags == SquareFlags::NorthEast ||
                    flags == (SquareFlags::All ^ SquareFlags::NorthEast)) {
                    x1 = x + sx * ValuesRatio(vals, 3, 2);
                    y1 = y + sy;
                    x2 = x + sx;
                    y2 = y + sy * ValuesRatio(vals, 1, 2);
                }
                else if (flags == SquareFlags::SouthEast ||
                    flags == (SquareFlags::All ^ SquareFlags::SouthEast)) {
                    x1 = x;
                    y1 = y+sy * ValuesRatio(vals, 0, 3);
                    x2 = x+sx * ValuesRatio(vals, 3, 2);
                    y2 = y+sy;
                }

                if (flags == SquareFlags::SouthWest) {
                    triangles.push_back({ x, y });
                    triangles.push_back({x1, y1 });
                    triangles.push_back({x2, y2 });
                }
                else if (flags == (SquareFlags::All ^ SquareFlags::SouthWest)) {
                    triangles.push_back({x1, y1 });
                    triangles.push_back({x+sx, y });
                    triangles.push_back({x+sx, y+sy });

                    triangles.push_back({x1, y1 });
                    triangles.push_back({x+sx, y+sy });
                    triangles.push_back({x2, y2 });

                    triangles.push_back({x2, y2 });
                    triangles.push_back({x+sx, y+sy });
                    triangles.push_back({x, y+sy });
                }
                else if (flags == SquareFlags::NorthWest) {
                    triangles.push_back({x+sx, y });
                    triangles.push_back({x1, y1 });
                    triangles.push_back({x2, y2 });
                }
                else if (flags == (SquareFlags::All ^ SquareFlags::NorthWest)) {
                    triangles.push_back({x, y });
                    triangles.push_back({x2, y2 });
                    triangles.push_back({x, y+sy });

                    triangles.push_back({x, y+sy });
                    triangles.push_back({x2, y2 });
                    triangles.push_back({x1, y1 });

                    triangles.push_back({x, y+sy });
                    triangles.push_back({x1, y1 });
                    triangles.push_back({x+sx, y+sy });
                }
                else if (flags == SquareFlags::NorthEast) {
                    triangles.push_back({x+sx, y+sy });
                    triangles.push_back({x1, y1 });
                    triangles.push_back({x2, y2 });
                }
                else if (flags == (SquareFlags::All ^ SquareFlags::NorthEast)) {
                    triangles.push_back({x, y });
                    triangles.push_back({x+sx, y });
                    triangles.push_back({x2, y2 });

                    triangles.push_back({x, y });
                    triangles.push_back({x2, y2 });
                    triangles.push_back({x1, y1 });

                    triangles.push_back({x, y });
                    triangles.push_back({x1, y1 });
                    triangles.push_back({x, y+sy });
                }
                else if (flags == SquareFlags::SouthEast) {
                    triangles.push_back({x, y+sy });
                    triangles.push_back({x1, y1 });
                    triangles.push_back({x2, y2 });
                }
                else if (flags == (SquareFlags::All ^ SquareFlags::SouthEast)) {
                    triangles.push_back({x, y });
                    triangles.push_back({x+sx, y });
                    triangles.push_back({x1, y1 });

                    triangles.push_back({x1, y1 });
                    triangles.push_back({x+sx, y });
                    triangles.push_back({x2, y2 });

                    triangles.push_back({x+sx, y });
                    triangles.push_back({x+sx, y+sy });
                    triangles.push_back({x2, y2 });
                }
            }
            else if (flags == (SquareFlags::SouthWest | SquareFlags::NorthWest)
                || flags == (SquareFlags::NorthWest | SquareFlags::NorthEast)
                || flags == (SquareFlags::NorthEast | SquareFlags::SouthEast)
                || flags == (SquareFlags::SouthEast | SquareFlags::SouthWest)) {
                // Half
                float x1 = 0.f, y1 = 0.f;
                float x2 = 0.f, y2 = 0.f;
                float x3 = 0.f, y3 = 0.f;
                float x4 = 0.f, y4 = 0.f;
                float sx = dX, sy = dY;

                if (flags == (SquareFlags::SouthWest | SquareFlags::NorthWest)) {
                    x1 = x;
                    y1 = y;
                    x2 = x + sx;
                    y2 = y;
                    x3 = x + sx;
                    y3 = y + sy * ValuesRatio(vals, 1, 2);
                    x4 = x;
                    y4 = y + sy * ValuesRatio(vals, 0, 3);
                }
                else if (flags == (SquareFlags::SouthEast | SquareFlags::NorthEast)) {
                    x1 = x;
                    y1 = y + sy * ValuesRatio(vals, 0, 3);
                    x2 = x + sx;
                    y2 = y + sy * ValuesRatio(vals, 1, 2);
                    x3 = x + sx;
                    y3 = y + sy;
                    x4 = x;
                    y4 = y + sy;
                }
                else if (flags == (SquareFlags::NorthWest | SquareFlags::NorthEast)) {
                    x1 = x + sx * ValuesRatio(vals, 0, 1);
                    y1 = y;
                    x2 = x + sx;
                    y2 = y;
                    x3 = x + sx;
                    y3 = y + sy;
                    x4 = x + sx * ValuesRatio(vals, 3, 2);
                    y4 = y + sy;
                }
                else if (flags == (SquareFlags::SouthWest | SquareFlags::SouthEast)) {
                    x1 = x;
                    y1 = y;
                    x2 = x+sx * ValuesRatio(vals, 0, 1);
                    y2 = y;
                    x3 = x+sx * ValuesRatio(vals, 3, 2);
                    y3 = y+sy;
                    x4 = x;
                    y4 = y+sy;
                }

                triangles.push_back({x1, y1 });
                triangles.push_back({x3, y3 });
                triangles.push_back({x4, y4 });

                triangles.push_back({x1, y1 });
                triangles.push_back({x2, y2 });
                triangles.push_back({x3, y3 });
            }
            else if (flags == (SquareFlags::SouthWest | SquareFlags::NorthEast) ||
                flags == (SquareFlags::NorthWest | SquareFlags::SouthEast)) {
                // Ambiguity
                float v = (vals.v[0] + vals.v[1] + vals.v[2] + vals.v[3]) / 4.0 - threshold;
                bool u = v > 0.0;

                float x1 = 0.f, y1 = 0.f;
                float x2 = 0.f, y2 = 0.f;
                float x3 = 0.f, y3 = 0.f;
                float x4 = 0.f, y4 = 0.f;
                float sx = dX, sy = dY;

                x1 = x+sx * ValuesRatio(vals, 0, 1);
                y1 = y;
                x2 = x+sx;
                y2 = y+sy * ValuesRatio(vals, 1, 2);
                x3 = x+sx * ValuesRatio(vals, 3, 2);
                y3 = y+sy;
                x4 = x;
                y4 = y+sy * ValuesRatio(vals, 0, 3);
            
                if (u) {
                    triangles.push_back({x1, y1 });
                    triangles.push_back({x2, y2 });
                    triangles.push_back({x3, y3 });

                    triangles.push_back({x1, y1 });
                    triangles.push_back({x3, y3 });
                    triangles.push_back({x4, y4 });
                }
            
                if (flags == (SquareFlags::SouthWest | SquareFlags::NorthEast)) {
                    triangles.push_back({x, y });
                    triangles.push_back({x1, y1 });
                    triangles.push_back({x4, y4 });

                    triangles.push_back({x+sx, y+sy });
                    triangles.push_back({x3, y3 });
                    triangles.push_back({x2, y2 });
                }
                else if (flags == (SquareFlags::NorthWest | SquareFlags::SouthEast)) {
                    triangles.push_back({x, y+sy });
                    triangles.push_back({x4, y4 });
                    triangles.push_back({x3, y3 });

                    triangles.push_back({x+sx, y });
                    triangles.push_back({x2, y2 });
                    triangles.push_back({x1, y1 });
                }
            }
            else if (flags == SquareFlags::All) {
                // Full fill
                float sx = dX, sy = dY;
            
                triangles.push_back({x, y });
                triangles.push_back({x+sx, y+sy });
                triangles.push_back({x, y+sy });

                triangles.push_back({x, y });
                triangles.push_back({x+sx, y });
                triangles.push_back({x+sx, y+sy });
            }
        }

#ifdef USE_OPENMP
#pragma omp critical
#endif
        {
            all_triangles.insert(
                all_triangles.end(),
                triangles.begin(),
                triangles.end());
        }
    }

    vbo_count = all_triangles.size();

    glBindBuffer(GL_ARRAY_BUFFER, vbo); LOGOPENGLERROR();
    glBufferData(GL_ARRAY_BUFFER, sizeof(hmm_vec2) * all_triangles.size(),
        all_triangles.data(), GL_STATIC_DRAW); LOGOPENGLERROR();

    LOGD << "Created parallel filled contour with " << all_triangles.size() / 3 << " triangles";

    return true;
}

void ContourFill::Render(const hmm_mat4& mvp,
                         double zoom,
                         const hmm_vec2& offset,
                         const FloatColor& c) {
    glUseProgram(program); LOGOPENGLERROR();
    glBindVertexArray(vao); LOGOPENGLERROR();

    glUniformMatrix4fv(u_mvp, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&mvp)); LOGOPENGLERROR();
    glUniform1f(u_zoom, zoom); LOGOPENGLERROR();
    glUniform2fv(u_ofs, 1, reinterpret_cast<const GLfloat*>(&offset)); LOGOPENGLERROR();
    glUniform2f(u_res, static_cast<GLfloat>(w), static_cast<GLfloat>(h)); LOGOPENGLERROR();
    glUniform4fv(u_color, 1, c.data()); LOGOPENGLERROR();

    glDrawArrays(GL_TRIANGLES, 0, vbo_count); LOGOPENGLERROR();

    glUseProgram(0); LOGOPENGLERROR();
    glBindVertexArray(0); LOGOPENGLERROR();
}
