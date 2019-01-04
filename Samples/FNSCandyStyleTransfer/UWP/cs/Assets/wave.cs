using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Media;
using Windows.Storage;
using Windows.Storage.Streams;
using Windows.AI.MachineLearning;
namespace SnapCandy
{
    
    public sealed class waveInput
    {
        public TensorFloat img_placeholder__0; // shape(-1,3,720,883)
    }
    
    public sealed class waveOutput
    {
        public TensorFloat add_37__0; // shape(-1,3,720,884)
    }
    
    public sealed class waveModel
    {
        private LearningModel model;
        private LearningModelSession session;
        private LearningModelBinding binding;
        public static async Task<waveModel> CreateFromStreamAsync(IRandomAccessStreamReference stream)
        {
            waveModel learningModel = new waveModel();
            learningModel.model = await LearningModel.LoadFromStreamAsync(stream);
            learningModel.session = new LearningModelSession(learningModel.model);
            learningModel.binding = new LearningModelBinding(learningModel.session);
            return learningModel;
        }
        public async Task<waveOutput> EvaluateAsync(waveInput input)
        {
            binding.Bind("img_placeholder__0", input.img_placeholder__0);
            var result = await session.EvaluateAsync(binding, "0");
            var output = new waveOutput();
            output.add_37__0 = result.Outputs["add_37__0"] as TensorFloat;
            return output;
        }
    }
}