/* Copyright 2019 eomain
   this program is licensed under the 2-clause BSD license
   see COPYING for the full license info

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
   OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stddef.h>

#include "ash/mem.h"
#include "ash/type.h"
#include "ash/util/queue.h"

#define QUEUE_DEFAULT_SIZE 8

static void queue_resize(struct queue *);

struct queue {
    void **data;
    size_t length;
    size_t capacity;
    size_t front;
    size_t rear;
};

struct queue *queue_from(size_t len)
{
    struct queue *queue;
    queue = ash_alloc(sizeof *queue);
    queue->data = ash_zalloc(len * sizeof *queue->data);
    queue->length = 0;
    queue->capacity = len;
    queue->front = 0;
    queue->rear = 0;
    return queue;
}

struct queue *queue_new(void)
{
    return queue_from(QUEUE_DEFAULT_SIZE);
}

void queue_destroy(struct queue *queue)
{
    if (queue->data)
        ash_free(queue->data);
    ash_free(queue);
}

void *queue_front(struct queue *queue)
{
    size_t index;
    index = queue->front;
    return queue->data[index];
}

void queue_enqueue(struct queue *queue, void *v)
{
    if ((queue->length > 0) &&
        (queue->front == queue->rear))
        queue_resize(queue);

    size_t index;
    index = queue->rear;
    queue->data[index] = v;
    queue->rear = ((queue->rear + 1) % queue->capacity);
    queue->length++;
}

void *queue_dequeue(struct queue *queue)
{
    void *v = NULL;
    size_t index;
    index = queue->front;
    v = queue->data[index];
    queue->data[index] = NULL;
    queue->front = ((queue->front + 1) % queue->capacity);
    queue->length--;
    return v;
}

size_t queue_len(struct queue *queue)
{
    return queue->length;
}

static void queue_resize(struct queue *queue)
{
    size_t len, size, index;
    void **buffer;

    index = queue->front;
    len = queue->length;
    size = (queue->capacity * 2);
    buffer = ash_zalloc((size * sizeof *queue->data));

    for (size_t i = 0; i < len; ++i) {
        buffer[i] = queue->data[index];
        index = ((index + 1) % queue->capacity);
    }

    ash_free(queue->data);
    queue->data = buffer;
    queue->capacity = size;
    queue->front = 0;
    queue->rear = len;
}
